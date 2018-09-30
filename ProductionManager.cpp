#include "ProductionManager.h"
#include "Blinkerbot.h"

ProductionManager::ProductionManager(BlinkerBot & bot): 
	blinkerBot(bot), baseManager(bot), productionQueue(bot), workerManager(bot), buildOrderManager(bot), 
	forwardPylon(nullptr), lastUsedWarpGate(nullptr),
	nextPylonLocation(Point2D(-1.0,-1.0)), lastProductionFrame(0),
	enemyHasCloak(false), reactedToRush(false), beingRushed(false), massLing(false)
{
}

void ProductionManager::initialise()
{
	//find the enemy race
	for (auto player : blinkerBot.Observation()->GetGameInfo().player_info)
	{
		if (player.player_id != blinkerBot.Observation()->GetPlayerID())
		{
			enemyRace = player.race_requested;
			productionQueue.setEnemyRace(enemyRace);
			buildOrderManager.setEnemyRace(enemyRace);
		}
	}

	for (auto unit : blinkerBot.Observation()->GetUnits())
	{
		if (UnitData::isWorker(unit))
		{
			workerManager.addWorker(unit);
		}
		else if (UnitData::isTownHall(unit))
		{
			addStructure(unit);
		}
	}
	productionQueue.initialiseQueue();
	baseManager.initialise();
	workerManager.initialise();
	buildOrderManager.initialise();
	rallyPoint = blinkerBot.Observation()->GetStartLocation();
	locateForwardPylonPoint();
}

void ProductionManager::onStep()
{
	/*
	//timer stuff
	std::clock_t start;
	double duration;
	start = std::clock();
	//timer stuff
	*/

	//printDebug();
	if (uint32_t(blinkerBot.Observation()->GetMinerals()) > DEADLOCK)
	{
		if (blinkerBot.Observation()->GetGameLoop() - lastProductionFrame > DEADLOCK)
		{
			//std::cerr << "attempting to break production deadlock" << std::endl;
			breakDeadlock();
		}
		else if (!productionQueue.includes(ABILITY_ID::BUILD_GATEWAY))
		{
			/*
			for (int i = 0; i < baseManager.getOurBases().size(); i++)
			{
				productionQueue.addItemHighPriority(ABILITY_ID::BUILD_GATEWAY);
			}
			*/
		}
	}

	//if the first pylon location hasn't been set yet, then let's set it
	if (nextPylonLocation == Point2D(-1, -1))
	{
		setNextPylonLocation();
	}

	//deal with the next item in the production queue
	produce(productionQueue.getNextItem());

	//update workers (called frequently because of scouting worker micro)
	workerManager.update();


	//the following don't need to be called so frequently
	if (blinkerBot.Observation()->GetGameLoop() % 30 == 0)
	{
		checkSupply();
		chronoBoost();

		//let's stop regular production briefly to make sure the next base gets started or a key unit gets trained
		if (!isBlocking(productionQueue.getNextItem().item) || armyStatus == Defend || blinkerBot.Observation()->GetMinerals() > 400)
		{
			trainUnits();
			trainWarp();
		}
		if (!isBlocking(productionQueue.getNextItem().item) || armyStatus == Defend)
		{
			trainSentries();

			//let's train some high tech units as long as our economy isn't too small
			if (workerManager.getWorkerCount() > 30)
			{
				trainColossus();
				trainHighTemplar();
			}
			if (workerManager.getWorkerCount() > 70)
			{
				trainVoidray();
			}

			upgrade();
		}
		else
		{
			//std::cerr << "cutting production for important item" << std::endl;
		}

		//if worker manager sends a warning that a base is mining out, build order manager needs to be informed
		buildOrderManager.receiveMiningOutSignal(workerManager.miningOut());
	}

	/*
	//timer stuff
	duration = (std::clock() - start) / (double)CLOCKS_PER_SEC * 1000;
	if (duration > 30)
	{
		std::cout << "ProductionManager duration: " << duration << '\n';
	}
	//timer stuff
	*/
}

/*
returns the closest pylon to the enemy base (used for warp ins)
*/
const Unit *ProductionManager::getClosestPylonToEnemyBase()
{
	const Unit *rallyPylon = nullptr;

	//first we want to make sure we have a base so we can compare distances
	if (!baseManager.getOurBases().empty())
	{
		//if we have a base, then we want to find the closest enemy base to that
		const Unit *closestEnemyBase = getClosestEnemyBase(baseManager.getOurBases().back().getBuildLocation());

		//if we've found an enemy base then find the closest one of our pylons to that
		if (closestEnemyBase)
		{
			rallyPylon = getClosestPylon(closestEnemyBase->pos);
		}
	}

	return rallyPylon;
}

/*
sets our rally point for warp ins
*/
void ProductionManager::setRallyPoint(Point2D point)
{
	rallyPoint = point;
}

/*
determines the type of thing we have at the front of the production queue and passes it to the appropriate function
*/
void ProductionManager::produce(BuildOrderItem nextBuildOrderItem)
{

	//check if the queue is empty. If it is, then let's generate a new goal and queue up some items
	if (nextBuildOrderItem.item == ABILITY_ID::INVALID)
	{
		productionQueue.generateMoreItems(buildOrderManager.generateGoal());
	}
	//deal with units
	else if (UnitData::isTrainableUnitType(nextBuildOrderItem.item))
	{
		train(nextBuildOrderItem);
	}
	//deal with researches
	else if (UnitData::isResearch(nextBuildOrderItem.item))
	{
		research(nextBuildOrderItem);
	}
	//deal with expansions
	else if (nextBuildOrderItem.item == ABILITY_ID::BUILD_NEXUS && canAfford(UNIT_TYPEID::PROTOSS_NEXUS) && 
		armyStatus != Defend && !(enemyRace != Race::Zerg && beingRushed))
	{
		expand();
	}
	//deal with structures
	else if (nextBuildOrderItem.item != ABILITY_ID::BUILD_NEXUS)
	{
		build(nextBuildOrderItem);
	}
}

/*
trains units automatically (independent of productionQueue)
*/
void ProductionManager::trainUnits()
{
	for (auto structure : structures)
	{
		//let's add an extra pylon if we have any unpowered production facilities
		if (UnitData::isStructure(structure) &&
			structure->unit_type != UNIT_TYPEID::PROTOSS_PYLON &&
			structure->unit_type != UNIT_TYPEID::PROTOSS_NEXUS &&
			structure->unit_type != UNIT_TYPEID::PROTOSS_ASSIMILATOR &&
			!structure->is_powered &&
			structure->build_progress == 1.0 &&
			productionQueue.getNextItem().item != ABILITY_ID::BUILD_PYLON)
		{
			productionQueue.addItemHighPriority(ABILITY_ID::BUILD_PYLON);
		}

		if (structure->unit_type == UNIT_TYPEID::PROTOSS_NEXUS && structure->orders.size() == 0 
			&& workerManager.getWorkerCount() <= (baseManager.getOurBases().size() * 22) && workerManager.getWorkerCount() <= 88)
		{
			blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAIN_PROBE);
		}
		else if (structure->unit_type == UNIT_TYPEID::PROTOSS_GATEWAY && structure->orders.size() == 0)
		{
			//if we have an idle gateway, try to make it into a warpgate
			if (buildOrderManager.alreadyResearched(ABILITY_ID::RESEARCH_WARPGATE) && structure->build_progress == 1.0)
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::MORPH_WARPGATE);		
				
				bool alreadyAdded = false;
				for (auto gate : warpGates)
				{
					if (structure == gate.warpgate)
					{
						alreadyAdded = true;
					}
				}
				if (!alreadyAdded)
				{
					warpGates.push_back(Warpgate(structure, 0, UNIT_TYPEID::INVALID));
				}
			}
			//otherwise train some units
			else if (!isBlocking(productionQueue.getNextItem().item) &&
				!massLing && 
				completedStructureExists(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) && 
				canAffordGas(UNIT_TYPEID::PROTOSS_STALKER))
			{
				if (canAfford(UNIT_TYPEID::PROTOSS_STALKER))
				{
					blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAIN_STALKER);
				}
			}
			else if (canAfford(UNIT_TYPEID::PROTOSS_ZEALOT))
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAIN_ZEALOT);
			}
		}
		else if (structure->unit_type == UNIT_TYPEID::PROTOSS_WARPGATE)
		{
			/*
			//find a warp in location
			Point2D location = Point2D(rallyPoint.x + GetRandomScalar() * 7.0f, rallyPoint.y + GetRandomScalar() * 7.0f);
			if (completedStructureExists(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) && canAffordGas(UNIT_TYPEID::PROTOSS_STALKER))
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAINWARP_STALKER, location);
				lastUsedWarpGate = structure;
			}
			else
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAINWARP_ZEALOT, location);
				lastUsedWarpGate = structure;
			}
			*/
		}
		else if (!isBlocking(productionQueue.getNextItem().item) && 
			structure->unit_type == UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY &&
			!completedStructureExists(UNIT_TYPEID::PROTOSS_ROBOTICSBAY)) 
		{
			if (structure->orders.size() == 0 && canAffordGas(UNIT_TYPEID::PROTOSS_IMMORTAL))
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAIN_IMMORTAL);
			}
		}
	}
}

/*
checks to see if any of our warpgates are idle and tries to warp in a unit
*/
void ProductionManager::trainWarp()
{
	int currentLoop = blinkerBot.Observation()->GetGameLoop();
	for (auto gate : warpGates)
	{
		if (currentLoop - gate.lastWarpInLoop >= UnitData::getWarpGateCoolDown(gate.lastTrainedType))
		{
			Point2D location = Point2D(rallyPoint.x + GetRandomScalar() * 7.0f, rallyPoint.y + GetRandomScalar() * 7.0f);
			if (!isBlocking(productionQueue.getNextItem().item) &&
				completedStructureExists(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) && 
				canAffordGas(UNIT_TYPEID::PROTOSS_STALKER))
			{
				if (canAfford(UNIT_TYPEID::PROTOSS_STALKER))
				{
					blinkerBot.Actions()->UnitCommand(gate.warpgate, ABILITY_ID::TRAINWARP_STALKER, location);
					lastUsedWarpGate = gate.warpgate;
					return;
				}
			}
			else if (canAfford(UNIT_TYPEID::PROTOSS_ZEALOT))
			{
				blinkerBot.Actions()->UnitCommand(gate.warpgate, ABILITY_ID::TRAINWARP_ZEALOT, location);
				lastUsedWarpGate = gate.warpgate;
				return;
			}
		}
	}
}

/*
takes a given BuildOrderItem and attempts to build a corresponding structure. Queues up required tech structures if necessary
*/
void ProductionManager::build(BuildOrderItem item)
{
	//if we're already at supply capacity, we don't want to make any more pylons
	//or if we already have this tech structure, we don't want to make a duplicate
	if ((item.item == ABILITY_ID::BUILD_PYLON && calculateSupplyCapacity() >= 200) ||
		(UnitData::isTechStructure(item.item) && structureExists(UnitData::getUnitTypeID(item.item))))
	{
		productionQueue.removeItem();
	}

	//let's check if we have the necessary tech to build this structure
	UnitTypeID requiredStructure = blinkerBot.Observation()->GetUnitTypeData()[UnitData::getUnitTypeID(item.item)].tech_requirement;
	bool isBuildable = false;
	if (requiredStructure == UNIT_TYPEID::INVALID)
	{
		isBuildable = true;
	}
	else
	{
		for (auto structure : structures)
		{
			if (structure->unit_type == requiredStructure)
			{
				isBuildable = true;
			}
		}
	}
	//if we don't then add what we need to the queue
	if (!isBuildable)
	{
		productionQueue.addItemHighPriority(blinkerBot.Observation()->GetUnitTypeData()[requiredStructure].ability_id);
	}
	else if (canAfford(UnitData::getUnitTypeID(item.item)))
	{
		buildStructure(item.item);
	}
}

//trains a unit from the production queue, or adds extra items to the queue in the event that we don't have the necessary tech
void ProductionManager::train(BuildOrderItem item)
{
	bool isStarted = false;
	bool isTrainable = false;
	const Unit *productionFacility = nullptr;
	UnitTypeID requiredStructure = UnitData::requiredStructure(item.item);

	//if the top command is to TRAIN_ a warpgate unit, but we already have warpgate, remove so it can be replaced by a TRAINWARP_ instead
	if (UnitData::isTrainedFromGateway(item.item) && warpGates.size() > 0)
	{
		productionQueue.removeItem();
	}

	//search our buildings
	for (auto structure : structures)
	{
		//check if we started the production facility
		for (auto order : structure->orders)
		{
			if (order.ability_id == item.item)
			{
				//if we've already started it then remove it from the to do list
				productionQueue.removeItem();
				lastProductionFrame = blinkerBot.Observation()->GetGameLoop();
				isStarted = true;
			}
		}
		//check if we have the necessary structure
		if (structure->unit_type == requiredStructure || requiredStructure == UNIT_TYPEID::INVALID)
		{
			productionFacility = structure;
			isTrainable = true;
		}
	}
	//if we are trying to build a colossus, we need to set the production facility to the robo facility rather than robo bay
	if (item.item == ABILITY_ID::TRAIN_COLOSSUS)
	{
		for (auto structure : structures)
		{
			if (structure->unit_type == UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY)
			{
				productionFacility = structure;
			}
		}
	}
	else if (item.item == ABILITY_ID::TRAIN_SENTRY)
	{
		for (auto structure : structures)
		{
			if (structure->unit_type == UNIT_TYPEID::PROTOSS_GATEWAY)
			{
				productionFacility = structure;
			}
		}
	}
	//if we don't have what we need, place what we need at the front of the production queue
	if (!isTrainable)
	{
		//std::cerr << UnitTypeToName(requiredStructure) << std::endl;
		productionQueue.addItemHighPriority(blinkerBot.Observation()->GetUnitTypeData()[requiredStructure].ability_id);
	}
	//if we haven't started it yet, then let's try
	else if (isTrainable && !isStarted)
	{
		//check if we have the necessary resources
		if (canAfford(UnitData::getUnitTypeID(item.item)))
		{
			//warp it in if possible
			if (UnitData::canWarpIn(item.item))
			{
				Point2D location = Point2D(rallyPoint.x + GetRandomScalar() * 7.0f, rallyPoint.y + GetRandomScalar() * 7.0f);
				if (warpGates.empty())
				{
					//std::cerr << "ProductionManager::train(): No warp gates found." << std::endl;
					productionQueue.removeItem();
				}
				else
				{
					const Unit *warpGate = getIdleWarpgate();
					if (warpGate)
					{
						blinkerBot.Actions()->UnitCommand(warpGate, item.item, location);
						lastUsedWarpGate = warpGate;
					}
					else
					{
						//std::cerr << "no idle warpgates" << std::endl;
					}
				}
			}
			//otherwise just train it
			else
			{
				blinkerBot.Actions()->UnitCommand(productionFacility, item.item);
			}
		}
	}
}

//researches a tech, or adds an item to the production queue if we don't have the necessary structure
void ProductionManager::research(BuildOrderItem item)
{
	bool isStarted = false;
	bool isResearchable = false;
	const Unit *researchStructure = nullptr;
	UnitTypeID requiredStructure = UnitData::requiredStructure(item.item);

	//make sure we don't already have this tech
	if (buildOrderManager.alreadyResearched(item.item))
	{
		productionQueue.removeItem();
	}

	//search our buildings
	for (auto structure : structures)
	{
		//check if we started the research already
		for (auto order : structure->orders)
		{
			if (order.ability_id == item.item || UnitData::isComparableUpgrade(order.ability_id, item.item))
			{
				//if we've already started it then remove it from the to do list
				productionQueue.removeItem();
				lastProductionFrame = blinkerBot.Observation()->GetGameLoop();
				isStarted = true;
			}
			
		}
		//check if we have the necessary structure
		if (structure->unit_type == requiredStructure)
		{
			researchStructure = structure;
			isResearchable = true;
		}
	}
	//if we don't have what we need, place what we need at the front of the production queue
	if (!isResearchable)
	{
		productionQueue.addItemHighPriority(blinkerBot.Observation()->GetUnitTypeData()[requiredStructure].ability_id);
	}
	//if the structure is already busy researching, let's just remove the new one from the queue and regenerate it later
	else if (!researchStructure->orders.empty())
	{
		productionQueue.removeItem();
	}
	//if we haven't started it yet, then let's try
	else if (!isStarted && canAfford(UnitData::getUpgradeID(item.item)))
	{
		blinkerBot.Actions()->UnitCommand(researchStructure, item.item);
	}
}

/*
let's the relevant managers know when a new structure is created
*/
void ProductionManager::addStructure(const Unit *unit)
{
	if (UnitData::isStructure(unit) && UnitData::isOurs(unit))
	{
		structures.insert(unit);

		if (unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
		{
			baseManager.addBase(unit);
			workerManager.addBase(unit);

			if (enemyRace == Race::Zerg && baseManager.getOurBases().size() > 2)
			{
				productionQueue.addItemHighPriority(ABILITY_ID::BUILD_PYLON);
			}
			else if (enemyRace != Race::Zerg && baseManager.getOurBases().size() > 1)
			{
				productionQueue.addItemHighPriority(ABILITY_ID::BUILD_PYLON);
			}
		}
	}
}

void ProductionManager::buildStructure(AbilityID structureToBuild)
{
	const Unit *builder = workerManager.getBuilder();
	if (!builder)
	{
		return;
	}

	if (structureToBuild == ABILITY_ID::BUILD_ASSIMILATOR)
	{
		//make sure we have a base that can have a base built at it
		if (!baseManager.getNextAvailableGas())
		{
			//if we don't then just remove this item from the queue
			productionQueue.removeItem();
			//we want to try adding it to the back of the queue again, but not if the queue is empty (because then we'll be right back here)
			if (productionQueue.getNextItem().item != ABILITY_ID::INVALID)
			{
				productionQueue.addItemLowPriority(ABILITY_ID::BUILD_ASSIMILATOR);
			}
		}
		//this is a workaround for the problem caused by baseManager storing unit pointers as snapshots which cannot be targeted with actions
		else if (baseManager.getNextAvailableGas()->display_type == Unit::DisplayType::Snapshot)
		{
			//if it's a snapshot pointer, then let's move to the location rather than issuing a build command
			blinkerBot.Actions()->UnitCommand(builder, ABILITY_ID::MOVE, baseManager.getNextAvailableGas()->pos);
			for (auto unit : blinkerBot.Observation()->GetUnits())
			{
				//if we find ourselves next to a geyser then build on it
				if (UnitData::isVespeneGeyser(unit) && Distance2D(unit->pos, builder->pos) < 2)
				{
					blinkerBot.Actions()->UnitCommand(builder, structureToBuild, unit);
					//we need to issue another command or the probe will wait for the gas to finish
					blinkerBot.Actions()->UnitCommand(builder, ABILITY_ID::STOP, true);
				}
			}
		}
		blinkerBot.Actions()->UnitCommand(builder, structureToBuild, baseManager.getNextAvailableGas());
		//we need to issue another command or the probe will wait for the gas to finish
		blinkerBot.Actions()->UnitCommand(builder, ABILITY_ID::STOP, true);

	}
	else
	{
		if (structureToBuild == ABILITY_ID::BUILD_PYLON)
		{
			if (pylons.empty())
			{
				blinkerBot.Actions()->UnitCommand(builder, structureToBuild, nextPylonLocation);
			}
			else
			{
				blinkerBot.Actions()->UnitCommand(builder, structureToBuild,
					Point2D(nextPylonLocation.x + GetRandomScalar() * 2, nextPylonLocation.y + GetRandomScalar() * 2));
			}
		}
		//shield batteries should be build in a location where they're likely to be used
		else if (structureToBuild == ABILITY_ID::BUILD_SHIELDBATTERY)
		{
			Point2D buildLocation;

			if (baseManager.getOurBases().empty())
			{
				buildLocation = blinkerBot.Observation()->GetStartLocation();
			}
			//if we only have one base then let's put them at the top of the ramp
			if (baseManager.getOurBases().size() == 1)
			{
				const Unit *rampPylon = getClosestPylon(baseManager.getMainFirstPylonPosition());
				if (Distance2D(rampPylon->pos, baseManager.getMainFirstPylonPosition()) < 2)
				{
					//buildLocation = baseManager.getFirstPylonPosition();
					buildLocation = baseManager.getMainRampTop();
				}
				else
				{
					buildLocation = getClosestPylon(getClosestBaseToEnemy()->pos)->pos;
				}
			}
			//otherwise let's choose the closest base to the enemy
			else
			{
				buildLocation = getClosestPylon(getClosestBaseToEnemy()->pos)->pos;
			}
			buildLocation = Point2D(buildLocation.x + GetRandomScalar() * 5.0f, buildLocation.y + GetRandomScalar() * 5.0f);
			blinkerBot.Actions()->UnitCommand(builder, structureToBuild, buildLocation);
		}
		//we want to make sure we have a cannon at each base for detection purposes
		else if (structureToBuild == ABILITY_ID::BUILD_PHOTONCANNON)
		{
			Point2D buildLocation;
			
			if (baseManager.getOurBases().empty())
			{
				const Unit *targetPylon = getLeastArtosisPylon();
				if (targetPylon)
				{
					buildLocation = Point2D(targetPylon->pos.x + GetRandomScalar() * 6.5f, targetPylon->pos.y + GetRandomScalar() * 6.5f);
				}
			}
			else
			{
				//if we're dealing with a rush in PvZ then we wanna put the extra cannons at the front
				if (beingRushed)
				{
					const Unit *targetPylon = getClosestPylon(baseManager.getOurBases().back().getBuildLocation());
					if (targetPylon)
					{
						buildLocation = Point2D(targetPylon->pos.x + GetRandomScalar() * 4.0f, targetPylon->pos.y + GetRandomScalar() * 4.0f);
					}
				}
				else
				{
					for (auto base : baseManager.getOurBases())
					{
						//check each base to see if a base has a cannon
						bool hasCannon = false;
						for (auto structure : structures)
						{
							if (Distance2D(base.getTownhall()->pos, structure->pos) < 10 && structure->unit_type == UNIT_TYPEID::PROTOSS_PHOTONCANNON)
							{
								hasCannon = true;
							}
						}
						//if we find a base with no cannon, let's get that location
						if (!hasCannon)
						{
							buildLocation = base.getTownhall()->pos;
						}
					}
					//find the closest pylon to that base, and build the cannon there
					const Unit *targetPylon = getClosestPylon(buildLocation);
					if (targetPylon)
					{
						buildLocation = Point2D(targetPylon->pos.x + GetRandomScalar() * 2.0f, targetPylon->pos.y + GetRandomScalar() * 2.0f);
					}
				}
			}
			blinkerBot.Actions()->UnitCommand(builder, structureToBuild, buildLocation);
		}
		else
		{
			Point2D buildLocation;
			const Unit *targetPylon = getLeastArtosisPylon();

			//otherwise find a pylon with some space and choose a random location within its power radius
			if (targetPylon)
			{
				if (enemyRace != Race::Terran)
				{
					if (!baseManager.mainFirstWallInPositionExists())
					{
						buildLocation = baseManager.getMainFirstWallInPosition();
					}
					else if (!baseManager.mainSecondWallInPositionExists())
					{
						buildLocation = baseManager.getMainSecondWallInPosition();
					}
					else
					{
						buildLocation = Point2D(targetPylon->pos.x + GetRandomScalar() * 6.5f, targetPylon->pos.y + GetRandomScalar() * 6.5f);
					}
				}
				else
				{
					buildLocation = Point2D(targetPylon->pos.x + GetRandomScalar() * 6.5f, targetPylon->pos.y + GetRandomScalar() * 6.5f);
				}
			}
			
			//check to make sure we aren't going to block our own base
			if (structureToBuild != ABILITY_ID::BUILD_NEXUS && Distance2D(buildLocation, baseManager.getNextBaseLocation()) < 5)
			{
				//std::cerr << "skipping to avoid blocking expansion location" << std::endl;
				return;
			}
			blinkerBot.Actions()->UnitCommand(builder, structureToBuild, buildLocation);
		}
	}
}

//returns the pylon powering the least number of structures
const Unit *ProductionManager::getLeastArtosisPylon()
{
	if (pylons.empty())
	{
		return nullptr;
	}

	//set some default values
	const Unit *leastArtosisPylon = *pylons.begin();
	int lowestPowering = int(structures.size());

	//check each pylon
	for (auto pylon : pylons)
	{
		//count the number of non-pylon structures in range
		int powering = 0;
		for (auto structure : structures)
		{
			if (structure->unit_type != UNIT_TYPEID::PROTOSS_PYLON && Distance2D(pylon->pos, structure->pos) < 6.5)
			{
				powering++;
			}
		}
		//if the number is lower than the previous lowest (and not a proxy pylon), record this one as lowest
		if (powering < lowestPowering && Distance2D(pylon->pos, forwardPylonPoint) > 10)
		{
			leastArtosisPylon = pylon;
			lowestPowering = powering;
		}
	}

	return leastArtosisPylon;
}

/*
determines the location of the next pylon to be constructed
*/
void ProductionManager::setNextPylonLocation()
{
	//if this is our first pylon
	if (pylons.empty())
	{
		//build it in the natural in PvZ
		if (enemyRace == Race::Zerg)
		{
			nextPylonLocation = baseManager.getMainFirstPylonPosition();
			//nextPylonLocation = baseManager.getNaturalFirstPylonPosition();
			return;
		}
		//put it at the top of our ramp in other matchups
		else
		{
			nextPylonLocation = baseManager.getMainFirstPylonPosition();
			return;
		}
	}
	else if (pylons.size() == 1 && enemyRace == Race::Zerg)
	{
		//nextPylonLocation = baseManager.getMainFirstPylonPosition();
		nextPylonLocation = baseManager.getNaturalFirstPylonPosition();
		return;
	}
	//if we need a forward pylon then build the pylon there
	if (enemyRace == Race::Zerg && !pylons.empty() && forwardPylon == nullptr && armyStatus == Attack)
	{
		//std::cerr << "making a forward pylon" << std::endl;
		nextPylonLocation = forwardPylonPoint;
	}

	//otherwise find an appropriate location
	else
	{
		Point2D buildLocation = blinkerBot.Observation()->GetStartLocation();

		//first let's check for an unpowered buildings that need a new pylon
		bool unpowered = false;
		for (auto structure : structures)
		{
			if (!structure->is_powered && 
				structure->unit_type != UNIT_TYPEID::PROTOSS_PYLON && structure->unit_type != UNIT_TYPEID::PROTOSS_NEXUS)
			{
				unpowered = true;
				buildLocation = structure->pos;
			}
		}
		if (unpowered)
		{
			std::vector<Point2D> buildGrid = getBuildGrid(buildLocation);

			for (auto point : buildGrid)
			{
				if (blinkerBot.Observation()->IsPlacable(point) && Distance2D(point, buildLocation) <= 5)
				{
					nextPylonLocation = point;
					return;
				}
			}		
		}

		if (baseManager.getOurBases().empty())
		{
			return;
		}
		else
		{
			//if we have started a new base which doesn't have any pylons near it yet
			const Unit *newBase = baseNeedsNearbyPylon();
			if (newBase)
			{
				//find a nearby point and build there
				bool found = false;
				std::vector<Point2D> buildGrid = getBuildGrid(newBase->pos);
				for (auto point : buildGrid)
				{
					if (Distance2D(point, newBase->pos) < NEARBYPYLONDIST)
					{
						found = true;
						buildLocation = point;
					}
				}
				if (found)
				{
					nextPylonLocation = buildLocation;
					return;
				}
			}

			//find the location of our most recent base
			bool townHallInProduction = false;
			for (auto unit : blinkerBot.Observation()->GetUnits())
			{
				if (UnitData::isOurs(unit) && unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS && unit->build_progress < 1.0)
				{
					buildLocation = unit->pos;
					townHallInProduction = true;
				}
			}
			if (!townHallInProduction)
			{
				buildLocation = baseManager.getOurBases().back().getBuildLocation();
			}
		}

		//get the grid of buildable locations for the area around that base
		std::vector<Point2D> buildGrid = getBuildGrid(buildLocation);

		//if there aren't any available locations at our current base, let's move on to the next one
		if (buildGrid.empty())
		{
			buildGrid = getBuildGrid(baseManager.getNextBaseLocation());
		}

		//select a random location from the available ones
		buildLocation = GetRandomEntry(buildGrid);
		/*
		if (Distance2D(buildLocation, baseManager.getNextBaseLocation()) < 6)
		{
			int count = 0;
			while (Distance2D(buildLocation, baseManager.getNextBaseLocation()) < 6 && count < 20)
			{
				buildLocation = GetRandomEntry(buildGrid);
				count++;
			}
		}
		*/

		nextPylonLocation = buildLocation;
	}
}

/*
build a structure at a specified point on the map
*/
void ProductionManager::buildStructure(AbilityID structureToBuild, Point2D target)
{
	const Unit *builder = workerManager.getBuilder();
	if (builder)
	{
		blinkerBot.Actions()->UnitCommand(builder, structureToBuild, target);
	}
}

/*
build a structure at a specified point on the map using the supplied unit
*/
void ProductionManager::buildStructure(const Unit *builder, AbilityID structureToBuild, Point2D target)
{
	if (builder)
	{
		blinkerBot.Actions()->UnitCommand(builder, structureToBuild, target);
	}
}

/*
updates the relevant sets and vectors when a new unit is created
*/
void ProductionManager::addNewUnit(const Unit *unit)
{
	//if the new unit matches the thing at the front of our queue, remove the front element
	//ability_id doesn't work for TRAINWARP units, so we need to check that separately
	if (blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].ability_id == productionQueue.getNextItem().item
		|| UnitData::getTrainWarpAbilityID(unit->unit_type) == productionQueue.getNextItem().item)
	{
		productionQueue.removeItem();
		lastProductionFrame = blinkerBot.Observation()->GetGameLoop();

		//check that no duplicate commands were issued and delete any if we find them
		if (productionQueue.getNextItem().item != blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].ability_id)
		{
			workerManager.checkForDuplicateBuildCommands(productionQueue.getNextItem().item);
		}
	}
	else if (blinkerBot.Observation()->GetGameLoop() > 1)
	{
		//if we didn't find it at the front of the queue, check the whole queue in case it has been updated while we were trying to produce
		if (productionQueue.findAndRemoveItem(blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].ability_id) ||
			productionQueue.findAndRemoveItem(UnitData::getTrainWarpAbilityID(unit->unit_type)))
		{
			lastProductionFrame = blinkerBot.Observation()->GetGameLoop();
		}
	}
	//if we made a warpgate unit then we want to update the cooldown on the gate we used
	if (UnitData::isWarpGateUnit(unit))
	{
		/*
		for (auto gate : warpGates)
		{
			if (gate.warpgate == lastUsedWarpGate)
			{
				std::cerr << "before update: " << gate.lastWarpInLoop << " " << UnitTypeToName(gate.lastTrainedType) << std::endl;
				gate.lastWarpInLoop = blinkerBot.Observation()->GetGameLoop();
				gate.lastTrainedType = unit->unit_type;
				std::cerr << "after update: " << gate.lastWarpInLoop << " " << UnitTypeToName(gate.lastTrainedType) << std::endl;
			}
		}
		*/
		
		bool found = false;
		for (std::vector<Warpgate>::iterator gate = warpGates.begin(); gate != warpGates.end();)
		{
			if ((*gate).warpgate == lastUsedWarpGate)
			{
				gate = warpGates.erase(gate);
				found = true;
			}
			else
			{
				gate++;
			}
		}
		if (found)
		{
			warpGates.push_back(Warpgate(lastUsedWarpGate, blinkerBot.Observation()->GetGameLoop(), unit->unit_type));
		}
	}
	for (auto gate : warpGates)
	{
		if (gate.warpgate == lastUsedWarpGate)
		{
			gate.lastWarpInLoop = blinkerBot.Observation()->GetGameLoop();
		}
	}

	//depending on the unit type, let's add it to the relevant group
	if (UnitData::isOurs(unit) && UnitData::isStructure(unit))
	{
		addStructure(unit);
	}
	if (UnitData::isOurs(unit) && unit->unit_type == UNIT_TYPEID::PROTOSS_PYLON)
	{
		pylons.insert(unit);
		if (Distance2D(unit->pos, forwardPylonPoint) < 30)
		{
			forwardPylon = unit;
		}
		setNextPylonLocation();
	}
	else if (UnitData::isOurs(unit) && unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE)
	{
		workerManager.addWorker(unit);
	}
	else if (UnitData::isOurs(unit) && unit->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR)
	{
		workerManager.addGas(unit);
		baseManager.addGas(unit);
		assimilators.insert(unit);
	}
	else if (unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS && blinkerBot.Observation()->GetGameLoop() > 1)
	{
		baseManager.removeNextBaseLocation();
	}
	else if (unit->unit_type == UNIT_TYPEID::PROTOSS_DARKTEMPLAR)
	{
		dts.insert(unit);
	}
	else if (unit->unit_type == UNIT_TYPEID::PROTOSS_OBSERVER)
	{
		observers.insert(unit);
	}
	else if (unit->unit_type == UNIT_TYPEID::PROTOSS_SHIELDBATTERY)
	{
		shieldBatteries.insert(unit);
	}
	else if (unit->unit_type == UNIT_TYPEID::PROTOSS_HIGHTEMPLAR)
	{
		hts.insert(unit);
	}
	else if (unit->unit_type == UNIT_TYPEID::PROTOSS_PHOTONCANNON)
	{
		photonCannons.insert(unit);
	}
	else if (unit->unit_type == UNIT_TYPEID::PROTOSS_SENTRY)
	{
		sentries.insert(unit);
	}
}

/*
updates the relevant sets and vectors when a unit dies
*/
void ProductionManager::onUnitDestroyed(const Unit *unit)
{
	//if we lose a gateway during a rush we want to remake it as a priority
	if (unit->unit_type == UNIT_TYPEID::PROTOSS_GATEWAY || unit->unit_type == UNIT_TYPEID::PROTOSS_WARPGATE)
	{
		productionQueue.addItemHighPriority(ABILITY_ID::BUILD_GATEWAY);
	}


	if (unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE)
	{
		workerManager.removeWorker(unit);
	}
	else if (unit->unit_type == UNIT_TYPEID::PROTOSS_DARKTEMPLAR)
	{
		dts.erase(unit);
	}
	else if (unit->unit_type == UNIT_TYPEID::PROTOSS_SHIELDBATTERY)
	{
		shieldBatteries.erase(unit);
	}
	else if (unit->unit_type == UNIT_TYPEID::PROTOSS_PHOTONCANNON)
	{
		photonCannons.erase(unit);
	}
	else if (unit->unit_type == UNIT_TYPEID::PROTOSS_OBSERVER)
	{
		observers.erase(unit);
	}
	else if (unit->unit_type == UNIT_TYPEID::PROTOSS_HIGHTEMPLAR)
	{
		hts.erase(unit);
	}
	else if (unit->unit_type == UNIT_TYPEID::PROTOSS_SENTRY)
	{
		sentries.erase(unit);
	}
	else if (unit->unit_type == UNIT_TYPEID::PROTOSS_WARPGATE)
	{
		for (std::vector<Warpgate>::iterator gate = warpGates.begin(); gate != warpGates.end();)
		{
			if ((*gate).warpgate == unit)
			{
				gate = warpGates.erase(gate);
			}
			else
			{
				++gate;
			}
		}
		//warpGates.erase(unit);
	}
	else
	{	
		for (std::set<const Unit *>::iterator structure = structures.begin(); structure != structures.end();)
		{
			if ((*structure) == unit)
			{
				structures.erase(*structure++);

				//if we lost something important, then we should recalculate our production queue
				productionQueue.clearQueue();
				productionQueue.generateMoreItems(buildOrderManager.generateGoal());

				//if we lost an expansion, make this point available to be expanded on again
				if (unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
				{
					baseManager.removeBase(unit);
					workerManager.removeBase(unit);
					baseManager.addAvailableBaseLocation(unit);
				}
				//if it was a gas then we need to update baseManager too
				else if (unit->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR)
				{
					workerManager.removeGas(unit);
					baseManager.removeGas(unit);
					assimilators.erase(unit);
				}
			}
			else
			{
				++structure;
			}
		}
		for (std::set<const Unit *>::iterator pylon = pylons.begin(); pylon != pylons.end();)
		{
			if ((*pylon) == unit)
			{
				pylons.erase(*pylon++);
			}
			else
			{
				++pylon;
			}
		}
	}
}

/*
receives the current ArmyStatus from ArmyManager.
*/
void ProductionManager::receiveArmyStatus(ArmyStatus status)
{
	armyStatus = status;
	workerManager.setArmyStatus(status);
}

/*
Determines an appropriate location to build a forward pylon for warping in
*/
void ProductionManager::locateForwardPylonPoint()
{
	//if we know where  the enemy base is, then let's build the pylon at a nearby base
	if (!baseManager.getAvailableBaseLocations().empty() && !enemyBases.empty())
	{
		Point2D enemyBase = (*enemyBases.begin())->pos;
		Base proxyBase = *baseManager.getAvailableBaseLocations().begin();
		for (auto base : baseManager.getAvailableBaseLocations())
		{
			//find the closest base that isn't TOO close
			if (Distance2D(enemyBase, base.getBuildLocation()) > 40 &&
				Distance2D(enemyBase, base.getBuildLocation()) < Distance2D(enemyBase, proxyBase.getBuildLocation()))
			{
				proxyBase = base;
			}
		}
		//forwardPylonPoint = proxyBase.getBuildLocation();
		forwardPylonPoint = Point2D(proxyBase.getBuildLocation().x + GetRandomScalar() * 5.0f, 
			proxyBase.getBuildLocation().y + GetRandomScalar() * 5.0f);
	}
	//if we don't know where the bases on the map are, or we don't know where the enemy base is, just put the pylon somewhere near the middle
	else
	{
		Point2D centre = Point2D(blinkerBot.Observation()->GetGameInfo().width / 2.0f, blinkerBot.Observation()->GetGameInfo().height / 2.0f);
		forwardPylonPoint = Point2D(centre.x + GetRandomScalar() * 25.0f, centre.y + GetRandomScalar() * 25.0f);
	}
}

/*
returns our closest pylon to a given point
*/
const Unit *ProductionManager::getClosestPylon(Point2D point)
{
	if (!pylons.empty())
	{
		const Unit *closest = (*pylons.begin());
		for (auto pylon : pylons)
		{
			if (Distance2D(pylon->pos, point) < Distance2D(closest->pos, point))
			{
				closest = pylon;
			}
		}
		return closest;
	}
	else
	{
		return nullptr;
	}
}

/*
Point2D ProductionManager::getRallyPoint()
{
	return rallyPoint;
}
*/

/*
calculate our current maximum supply capacity
*/
float ProductionManager::calculateSupplyCapacity()
{
	float supplyCapacity = 0;

	for (auto structure : structures)
	{
		if (structure->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
		{
			supplyCapacity += 15;
		}
		if (structure->unit_type == UNIT_TYPEID::PROTOSS_PYLON)
		{
			supplyCapacity += 8;
		}
	}
	return supplyCapacity;
}

/*
checks our current supply status and queues up extra pylons if necessary
*/
void ProductionManager::checkSupply()
{
	float supplyCapacity = calculateSupplyCapacity();
	float supplyUsed = 0;
	bool pylonAlreadyInProduction = false;

	//calculate how much supply we have
	for (auto unit : blinkerBot.Observation()->GetUnits())
	{
		if (UnitData::isOurs(unit))
		{
			//std::cerr << UnitTypeToName(unit->unit_type) << blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].food_required << std::endl;
			supplyUsed += blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].food_required;
		}
	}
	//check we don't already have a pylon in production
	for (auto pylon : pylons)
	{
		//std::cerr << "checkin plyons" << std::endl;
		if (pylon->build_progress < 1.0)
		{
			pylonAlreadyInProduction = true;
		}
	}
	//if we're nearly blocked, the next item in the queue is not a pylon, and we don't have one already building, then make a new one
	if (supplyCapacity - supplyUsed < 5.0f && productionQueue.getNextItem().item != ABILITY_ID::BUILD_PYLON && !pylonAlreadyInProduction)
	{
		if (baseManager.getOurBases().empty())
		{
			productionQueue.addItemHighPriority(ABILITY_ID::BUILD_PYLON);
		}
		else
		{
			//add a pylon for each base we have
			for (int i = 0; i < baseManager.getOurBases().size(); i++)
			{
				productionQueue.addItemHighPriority(ABILITY_ID::BUILD_PYLON);
			}
		}
	}
}

/*
expand to the next closest base location
*/
void ProductionManager::expand()
{
	Point2D nextBase = baseManager.getNextBaseLocation();
	
	if (blinkerBot.Query()->Placement(ABILITY_ID::BUILD_NEXUS, nextBase, workerManager.getBuilder()))
	{
		buildStructure(ABILITY_ID::BUILD_NEXUS, nextBase);
	}
	else
	{
		buildStructure(ABILITY_ID::BUILD_NEXUS, Point2D(nextBase.x + GetRandomScalar(), nextBase.y + GetRandomScalar()));
	}
}

/*
called when we find a new enemy base
*/
void ProductionManager::addEnemyBase(const Unit *unit)
{
	enemyBases.insert(unit);
	workerManager.addEnemyMain(unit);

	//if our opponent chose random, we can update its race once we've scouted its base
	if (enemyRace == Race::Random && !enemyBases.empty())
	{
		enemyRace = blinkerBot.Observation()->GetUnitTypeData()[(*enemyBases.begin())->unit_type].race;
		productionQueue.setEnemyRace(enemyRace);
		buildOrderManager.setEnemyRace(enemyRace);
	}
}

/*
called when we destroy an enemy base
*/
void ProductionManager::removeEnemyBase(const Unit *unit)
{
	for (std::set<const Unit *>::iterator base = enemyBases.begin(); base != enemyBases.end();)
	{
		if ((*base) == unit)
		{
			enemyBases.erase(base++);
		}
		else
		{
			++base;
		}
	}
}

/*
pass finished upgrades to BuildOrderManager to keep track of
*/
void ProductionManager::onUpgradeComplete(UpgradeID upgrade)
{
	buildOrderManager.onUpgradeComplete(upgrade);
}

/*
find our nexuses and use chronoboost if they have enough energy
*/
void ProductionManager::chronoBoost()
{
	for (auto structure : structures)
	{
		if (structure->unit_type == UNIT_TYPEID::PROTOSS_NEXUS && structure->energy > 50)
		{
			//find a the most important thing in production to chronoboost
			const Unit * target = getHighestPriorityInProduction(getCurrentlyInProduction());
			if (target)
			{
				//3755 = new chronoboost. EFFECT_CHRONOBOOST is the old pre-LOTV one.
				blinkerBot.Actions()->UnitCommand(structure, AbilityID(3755), target);
			}
		}
	}
}

/*
return a set of pairs representing all the things that are currently in production at our structures along with a pointer to the corresponding structure
*/
std::set<std::pair<AbilityID, const Unit *>> ProductionManager::getCurrentlyInProduction()
{
	std::set<std::pair<AbilityID, const Unit *>> inProduction;
	
	for (auto structure : structures)
	{
		for (auto order : structure->orders)
		{
			inProduction.insert(std::make_pair(order.ability_id, structure));
		}
	}
	return inProduction;
}

/*
take the set of things that are currently in production, and return a pointer to the structure which is producing the most important one
*/
const Unit * ProductionManager::getHighestPriorityInProduction(std::set<std::pair<AbilityID, const Unit *>> inProduction)
{
	std::pair<AbilityID, const Unit *> highest = std::make_pair(ABILITY_ID::INVALID, nullptr);

	if (inProduction.size() > 0)
	{
		//find the highest priority item
		highest = *inProduction.begin();
		for (auto item : inProduction)
		{
			if (checkPriority(item.first) < checkPriority(highest.first))
			{
				highest = item;
			}
		}
	}
	//if nothing is in production then just chrono a warpgate instead
	else
	{
		for (auto structure : structures)
		{
			if (structure->unit_type == UNIT_TYPEID::PROTOSS_WARPGATE)
			{
				return structure;
			}
		}
	}

	return highest.second;
}

/*
return an int representing the priority of the ability (low value = high priority)
*/
int ProductionManager::checkPriority(ABILITY_ID ability)
{
	//chronoboost priority changes when we are being rushed
	if (beingRushed)
	{
		switch (ability)
		{
		case ABILITY_ID::INVALID:
			return 100;
			break;
		case ABILITY_ID::TRAIN_STALKER:
		case ABILITY_ID::TRAIN_ZEALOT:
			return 1;
			break;
		case ABILITY_ID::RESEARCH_WARPGATE:
		case ABILITY_ID::RESEARCH_BLINK:
			return 2;
			break;
		default:
			return 3;
			break;
		}
	}
	//standard priority order
	else
	{
		switch (ability)
		{
		case ABILITY_ID::INVALID:
			return 100;
			break;
		case ABILITY_ID::RESEARCH_WARPGATE:
		case ABILITY_ID::RESEARCH_BLINK:
		case ABILITY_ID::TRAIN_OBSERVER:
		case ABILITY_ID::RESEARCH_PSISTORM:
			return 1;
			break;
		case ABILITY_ID::TRAIN_COLOSSUS:
		case ABILITY_ID::TRAIN_IMMORTAL:
		case ABILITY_ID::TRAIN_VOIDRAY:
			return 2;
			break;
		case ABILITY_ID::RESEARCH_EXTENDEDTHERMALLANCE:
		case ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS:
		case ABILITY_ID::RESEARCH_PROTOSSGROUNDARMOR:
		case ABILITY_ID::RESEARCH_PROTOSSSHIELDS:
			return 3;
			break;
		case ABILITY_ID::TRAIN_STALKER:
			return 4;
		default:
			return 5;
			break;
		}
	}
}

/*
if army manager sends us a signal alerting us of undetected cloak, let's make some detection
*/
void ProductionManager::receiveCloakSignal(bool signal)
{
	buildOrderManager.receiveCloakSignal(signal);

	//check if we have the ability to produce detectors
	bool detectionProducable = false;
	for (auto structure : structures)
	{
		if (structure->unit_type == UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY && structure->build_progress == 1.0)
		{
			detectionProducable = true;
		}
	}

	//check if we already started producing
	bool alreadyConstructing = false;
	bool alreadyTraining = false;
	for (auto item : getCurrentlyInProduction())
	{
		if (item.first == ABILITY_ID::TRAIN_OBSERVER)
		{
			alreadyTraining = true;
		}
		else if (item.first == ABILITY_ID::BUILD_ROBOTICSFACILITY)
		{
			alreadyConstructing = true;
		}
	}

	//(signal is passed via Army Manager)
	if (signal)
	{
		enemyHasCloak = true;

		//if we don't already have a robo facility on the way, let's start one
		if (!detectionProducable && !alreadyConstructing)
		{
			if (!productionQueue.includes(ABILITY_ID::BUILD_ROBOTICSFACILITY))
			{
				productionQueue.addItemHighPriority(ABILITY_ID::BUILD_ROBOTICSFACILITY);
			}
		}
	}

	//if we can make observers but we don't have one, make one
	if (detectionProducable && !alreadyTraining && observers.empty())
	{
		if (!productionQueue.includes(ABILITY_ID::TRAIN_OBSERVER))
		{
			productionQueue.addItemHighPriority(ABILITY_ID::TRAIN_OBSERVER);
		}
	}
}

/*
see how many of our bases are still have sufficient available minerals
*/
int ProductionManager::calculateMiningBases()
{
	checkMineralVisibility();

	int miningBases = 0;
	for (auto base : baseManager.getOurBases())
	{
		int mineralCount = 0;
		for (auto mineral : base.getMinerals())
		{
			if (mineral->display_type == Unit::DisplayType::Snapshot && blinkerBot.Observation()->GetGameLoop() % 30 == 0)
			{
				//std::cerr << "this mineral node is not visible" << std::endl;
			}
			if (mineral->mineral_contents > 100)
			{
				mineralCount++;
			}
		}
		if (mineralCount > 3)
		{
			miningBases++;
		}
	}
	return miningBases;
}

/*
checks if any of our bases have minerals or geysers stored as snapshots (fog of war), which will make them impossible to interact with
*/
void ProductionManager::checkMineralVisibility()
{
	for (auto base : baseManager.getOurBases())
	{
		//find if anything is stored as snapshot
		bool snapshot = false;
		for (auto mineral : base.getMinerals())
		{
			if (mineral->display_type == Unit::DisplayType::Snapshot)
			{
				snapshot = true;
			}
		}
		for (auto geyser : base.getGeysers())
		{
			if (geyser->display_type == Unit::DisplayType::Snapshot)
			{
				snapshot = true;
			}
		}
		bool visible = false;
		for (auto unit : blinkerBot.Observation()->GetUnits())
		{
			if ((Distance2D(unit->pos, base.getBuildLocation()) < 15) 
				&& UnitData::isMinerals(unit) || UnitData::isVespeneGeyser(unit)
				&& unit->display_type == Unit::DisplayType::Visible)
			{
				visible = true;
			}
		}
		//if we have nodes stores as snapshot, but they are currently visible, then find the base and update it in base manager
		if (snapshot && visible)
		{
			for (auto unit : blinkerBot.Observation()->GetUnits())
			{
				if (UnitData::isOurs(unit)
					&& Distance2D(unit->pos, base.getBuildLocation()) < 15
					&& unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
				{
					baseManager.updateCompletedBase(unit);
				}
			}
		}
	}
}

void ProductionManager::printDebug()
{
	/*
	for (int w = 0; w != int(blinkerBot.Observation()->GetGameInfo().width); w++)
	{
		for (int h = 0; h != int(blinkerBot.Observation()->GetGameInfo().height); h++)
		{
			if (blinkerBot.Query()->Placement(ABILITY_ID::BUILD_PYLON, Point2D(float(w), float(h))))
			{
				Point3D point = Point3D(float(w), float(h), blinkerBot.Observation()->GetStartLocation().z + 1);
				blinkerBot.Debug()->DebugBoxOut(point, Point3D(float(point.x + 1), float(point.y + 1), point.z), Colors::Purple);
			}
		}
	}
	*/
	blinkerBot.Debug()->DebugBoxOut(Point3D(nextPylonLocation.x, nextPylonLocation.y, blinkerBot.Observation()->GetStartLocation().z + 1),
		Point3D(nextPylonLocation.x + 2, nextPylonLocation.y + 2, blinkerBot.Observation()->GetStartLocation().z - 3));
	blinkerBot.Debug()->SendDebug();

}

/*
generates a 30x30 grid of buildable locations around a centre point
*/
std::vector<Point2D> ProductionManager::getBuildGrid(Point2D centre)
{
	std::vector<QueryInterface::PlacementQuery> queries;
	std::vector<Point2D> buildGrid;

	//first we set the minimum and maximum values for the search area
	float minX = centre.x - 15;
	float minY = centre.y - 15;
	float maxX = centre.x + 15;
	float maxY = centre.y + 15;

	if (minX < 0)
	{
		minX = 0;
	}
	if (minY < 0)
	{
		minY = 0;
	}
	if (maxX > blinkerBot.Observation()->GetGameInfo().width)
	{
		maxX = float(blinkerBot.Observation()->GetGameInfo().width);
	}
	if (maxY > blinkerBot.Observation()->GetGameInfo().width)
	{
		maxY = float(blinkerBot.Observation()->GetGameInfo().width);
	}

	//load up the vector with a query for each point
	for (float x = minX; x <= maxX; x++)
	{
		for (float y = minY; y <= maxY; y++)
		{
			queries.push_back(QueryInterface::PlacementQuery(ABILITY_ID::BUILD_PYLON, Point2D(x, y)));
			//if (blinkerBot.Observation()->IsPlacable(Point2D(x, y)))
			//{
				buildGrid.push_back(Point2D(x, y));
			//}
		}
	}
	
	//query each position
	std::vector<bool> output = blinkerBot.Query()->Placement(queries);

	if (!output.empty())
	{
		//if a point is unbuildable, set its location to 0:0 so we know it doesn't work
		for (int i = 0; i != output.size() - 1; ++i)
		{
			//std::cerr << i << ": " << output[i] << std::endl;
			if (output[i] == false)
			{
				buildGrid[i] = Point2D(0, 0);
			}
		}
	}
	//go through the vector and remove all the unbuildable (0:0) points
	for (std::vector<Point2D>::iterator point = buildGrid.begin(); point != buildGrid.end();)
	{
		if (*point == Point2D(0, 0) || Distance2D(*point, baseManager.getNextBaseLocation()) < 6)
		{
			point = buildGrid.erase(point);
		}
		else
		{
			++point;
		}
	}
	for (std::vector<Point2D>::iterator point = buildGrid.begin(); point != buildGrid.end();)
	{
		bool tooClose = false;

		//let's make sure it's not too close to one of our buildings
		for (auto structure : structures)
		{
			if (Distance2D(*point, structure->pos) < 4)
			{
				//std::cerr << "removing a location for being too close to a structure" << std::endl;
				tooClose = true;
			}
		}

		//let's make sure it's not too close to our resources (we don't wanna block any mineral or gas lines
		for (auto resource : blinkerBot.Observation()->GetUnits())
		{
			if ((UnitData::isMinerals(resource) || UnitData::isVespeneGeyser(resource)) && Distance2D(*point, resource->pos) < 4)
			{
				//std::cerr << "removing a location for being too close to a mineral/gas" << std::endl;
				tooClose = true;
			}
		}
		if (tooClose)
		{
			point = buildGrid.erase(point);
		}
		else
		{
			++point;
		}
	}

	//printBuildGrid(buildGrid);
	return buildGrid;
}

/*
prints out a set of squares representing buildable points
*/
void ProductionManager::printBuildGrid(std::vector<Point2D> buildGrid)
{
	for (auto point : buildGrid)
	{
		Point3D box = Point3D(point.x, point.y, blinkerBot.Observation()->GetStartLocation().z + 1);
		blinkerBot.Debug()->DebugBoxOut(box, Point3D(box.x + 1, box.y + 1, box.z), Colors::Purple);
	}
	blinkerBot.Debug()->SendDebug();
}

/*
returns the closest enemy base to a given point
*/
const Unit *ProductionManager::getClosestEnemyBase(Point2D point)
{
	if (enemyBases.empty())
	{
		return nullptr;
	}
	else
	{
		const Unit *closestBase = *enemyBases.begin();
		for (auto base : enemyBases)
		{
			if (Distance2D(base->pos, point) < Distance2D(closestBase->pos, point))
			{
				closestBase = base;
			}
		}
		return closestBase;
	}
}

/*
tries to reset in the case that the production queue gets stuck for some reason
*/
void ProductionManager::breakDeadlock()
{
	//if we were trying to expand, then let's find a different location
	if (productionQueue.getNextItem().item == ABILITY_ID::BUILD_NEXUS)
	{
		baseManager.removeNextBaseLocation();
	}
	//if we were trying to build a pylon, then get a new random location
	if (productionQueue.getNextItem().item == ABILITY_ID::BUILD_PYLON)
	{
		locateForwardPylonPoint();
		setNextPylonLocation();
	}
	//let's delete whatever was in our current queue, and generate a new one
	productionQueue.clearQueue();
	productionQueue.generateMoreItems(buildOrderManager.generateGoal());
	lastProductionFrame = blinkerBot.Observation()->GetGameLoop();
}

/*
returns true if we already have one of this type of structure (completed or in progress)
*/
bool ProductionManager::structureExists(UnitTypeID structure)
{
	for (auto ourStructure : structures)
	{
		if (ourStructure->unit_type == structure)
		{
			return true;
		}
	}
	return false;
}

/*
returns true if we already have one of this type of structure completed
*/
bool ProductionManager::completedStructureExists(UnitTypeID structure)
{
	for (auto ourStructure : structures)
	{
		if (ourStructure->unit_type == structure && ourStructure->build_progress == 1.0)
		{
			return true;
		}
	}
	return false;
}

/*
makes a dark shrine and dts if we don't have them already
*/
void ProductionManager::darkShrine(bool signal)
{
	//signal is passed via Army Manager telling us when we need to start a dark shrine
	if (signal)
	{
		//if we haven't yet started a dark shrine, let's do that and add a dt to the end of the queue for later
		if (!structureExists(UNIT_TYPEID::PROTOSS_DARKSHRINE) && !productionQueue.includes(ABILITY_ID::BUILD_DARKSHRINE))
		{
			productionQueue.addItemHighPriority(ABILITY_ID::BUILD_DARKSHRINE);
			productionQueue.addItemLowPriority(ABILITY_ID::TRAINWARP_DARKTEMPLAR);
		}
	}

	//if we already have a dark shrine, then just queue up a dt if don't have one
	if (completedStructureExists(UNIT_TYPEID::PROTOSS_DARKSHRINE) && dts.empty() 
		&& !productionQueue.includes(ABILITY_ID::TRAINWARP_DARKTEMPLAR) && blinkerBot.Observation()->GetFoodUsed() < 190)
	{
		productionQueue.addItemHighPriority(ABILITY_ID::TRAINWARP_DARKTEMPLAR);
	}
}

/*
returns true if we have enough minerals and gas to make the given unit type
*/
bool ProductionManager::canAfford(UnitTypeID unit)
{
	/*
	std::cerr << UnitTypeToName(unit) << " " << blinkerBot.Observation()->GetUnitTypeData()[unit].mineral_cost 
		<< "/" << blinkerBot.Observation()->GetUnitTypeData()[unit].vespene_cost << std::endl;
	*/
	if (blinkerBot.Observation()->GetVespene() >= blinkerBot.Observation()->GetUnitTypeData()[unit].vespene_cost &&
		blinkerBot.Observation()->GetMinerals() >= blinkerBot.Observation()->GetUnitTypeData()[unit].mineral_cost)
	{
		return true;
	}
	else
	{
		//std::cerr << "we can't afford a " << UnitTypeToName(unit) << std::endl;
		return false;
	}
}

/*
returns true if we have enough minerals and gas to research the upgrade
*/
bool ProductionManager::canAfford(UpgradeID upgrade)
{
	if (uint32_t(blinkerBot.Observation()->GetVespene()) >= blinkerBot.Observation()->GetUpgradeData()[upgrade].vespene_cost &&
		uint32_t(blinkerBot.Observation()->GetMinerals()) >= blinkerBot.Observation()->GetUpgradeData()[upgrade].mineral_cost)
	{
		//std::cerr << "we can afford a " << UpgradeIDToName(upgrade) << std::endl;
		return true;
	}
	else
	{
		//std::cerr << "we can't afford a " << UpgradeIDToName(upgrade) << std::endl;
		return false;
	}
}

/*
returns true if we have enough gas to make the given unit type
*/
bool ProductionManager::canAffordGas(UnitTypeID unit)
{
	if (blinkerBot.Observation()->GetVespene() >= blinkerBot.Observation()->GetUnitTypeData()[unit].vespene_cost)
	{
		return true;
	}
	else
	{
		//std::cerr << "we can't afford a " << UnitTypeToName(unit) << std::endl;
		return false;
	}
}

/*
returns true if the ability is a high priority ability (so that we know to block other production until it starts)
*/
bool ProductionManager::isBlocking(AbilityID ability)
{
	//some important buildings and units are blocking
	if (ability == ABILITY_ID::BUILD_NEXUS || ability == ABILITY_ID::BUILD_ROBOTICSFACILITY || 
		(completedStructureExists(UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL) && ability == ABILITY_ID::BUILD_TEMPLARARCHIVE) ||
		(completedStructureExists(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY) && 
			completedStructureExists(UNIT_TYPEID::PROTOSS_ROBOTICSBAY) && ability == ABILITY_ID::TRAIN_COLOSSUS) ||
		(completedStructureExists(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY) && ability == ABILITY_ID::TRAIN_IMMORTAL) ||
		(completedStructureExists(UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE) && ability == ABILITY_ID::TRAINWARP_HIGHTEMPLAR) ||
		(completedStructureExists(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY) && ability == ABILITY_ID::BUILD_ROBOTICSBAY) ||
		(completedStructureExists(UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL) && ability == ABILITY_ID::BUILD_DARKSHRINE) ||
		(completedStructureExists(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY) && ability == ABILITY_ID::TRAIN_OBSERVER) ||
		(completedStructureExists(UNIT_TYPEID::PROTOSS_DARKSHRINE) && ability == ABILITY_ID::TRAINWARP_DARKTEMPLAR) ||
		(completedStructureExists(UNIT_TYPEID::PROTOSS_STARGATE) && ability == ABILITY_ID::TRAIN_VOIDRAY))
	{
		return true;
	}
	//key technologies are also blocking as long as we have the necessary tech to start them
	else if (buildOrderManager.isKeyTech(ability) && completedStructureExists(UnitData::requiredStructure(ability)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
adds colossuses to the production queue
*/
void ProductionManager::trainColossus()
{
	for (auto structure : structures)
	{
		//if we have an idle completed robo facility and can train colossus, add one to the queue
		if (structure->unit_type == UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY &&
			structure->build_progress == 1.0 &&
			structure->orders.empty() &&
			completedStructureExists(UNIT_TYPEID::PROTOSS_ROBOTICSBAY) &&
			!productionQueue.includes(ABILITY_ID::TRAIN_COLOSSUS)
			&& blinkerBot.Observation()->GetFoodUsed() < 190)
		{
			productionQueue.addItemHighPriority(ABILITY_ID::TRAIN_COLOSSUS);
		}
	}
}

/*
adds high templars to the production queue
*/
void ProductionManager::trainHighTemplar()
{
	//if we can train HTs and we don't already have plenty, let's add another one to the queue
	if (armyStatus != Defend && completedStructureExists(UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE) && (hts.size() < baseManager.getOurBases().size() * 2)
		&& !productionQueue.includes(ABILITY_ID::TRAINWARP_HIGHTEMPLAR) && !productionQueue.includes(ABILITY_ID::RESEARCH_PSISTORM) 
		&& blinkerBot.Observation()->GetFoodUsed() < 190)
	{
		productionQueue.addItemHighPriority(ABILITY_ID::TRAINWARP_HIGHTEMPLAR);
	}
}


/*
finds the base with the shortest distance to an enemy base
*/
const Unit *ProductionManager::getClosestBaseToEnemy()
{
	if (baseManager.getOurBases().empty())
	{
		return nullptr;
	}

	const Unit *closestBase = GetRandomEntry(baseManager.getOurBases()).getTownhall();

	if (enemyBases.empty())
	{
		return closestBase;
	}

	for (auto base : baseManager.getOurBases())
	{
		if (Distance2D(base.getBuildLocation(), getClosestEnemyBase(base.getBuildLocation())->pos) < 
			Distance2D(closestBase->pos, getClosestEnemyBase(closestBase->pos)->pos))
		{
			closestBase = base.getTownhall();
		}
	}

	return closestBase;
}

/*
returns the closest pylon to whichever of our bases is closest to the enemy
*/
Point2D ProductionManager::getDefensivePosition()
{
	//if we have only one base, let's defend the top of our ramp
	if (baseManager.getOurBases().size() < 2)
	{
		//return baseManager.getMainRampTop();
		return baseManager.getMainFirstPylonPosition();
	}
	else
	{
		//otherwise let's find a pylon near our most vulnerable base and rally to that
		const Unit *base = getClosestBaseToEnemy();
		const Unit *target;

		if (base)
		{
			target = getClosestPylon(base->pos);
		}
		else
		{
			//if we don't have any bases then just use the closest pylon
			target = getClosestPylonToEnemyBase();
		}
		
		if (target)
		{
			return target->pos;
		}
		else
		{
			//if there's no pylon to rally to, just use the start location
			return blinkerBot.Observation()->GetStartLocation();
		}
	}
}

/*
if it seems like we're being rushed, alter our build order and add shield batteries if possible
*/
void ProductionManager::receiveRushSignal(bool signal)
{
	if (signal)
	{
		beingRushed = true;

		//protoss or terran response
		//if (enemyRace != Race::Zerg)
		//{
			//if this is the first we've heard about it, cancel the current build order and get a defensive one
			if (!reactedToRush)
			{
				productionQueue.clearQueue();
				productionQueue.generateMoreItems(buildOrderManager.generateRushDefenceGoal());
				reactedToRush = true;
			}
		//}
		//if we can make shield batteries, let's do it
		if (completedStructureExists(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE))
		{
			int extraShieldBatteries = 2 - shieldBatteries.size();
			bool shieldBatteryInProduction = false;

			if (extraShieldBatteries > 0 && !productionQueue.includes(ABILITY_ID::BUILD_SHIELDBATTERY))
			{
				for (int i = 0; i < extraShieldBatteries; i++)
				{
					productionQueue.addItemHighPriority(ABILITY_ID::BUILD_SHIELDBATTERY);
				}
			}
		}
		//if we can make cannons, do that
		if (completedStructureExists(UNIT_TYPEID::PROTOSS_FORGE))
		{
			int extraCannons = 2 - photonCannons.size();

			if (extraCannons > 0 && !productionQueue.includes(ABILITY_ID::BUILD_PHOTONCANNON))
			{
				for (int i = 0; i < extraCannons; i++)
				{
					productionQueue.addItemHighPriority(ABILITY_ID::BUILD_PHOTONCANNON);
				}
			}
		}
	}
	else
	{
		beingRushed = false;
	}
}

/*
returns true if the enemy is trying to cause a draw
*/
bool ProductionManager::cleanUp()
{
	//we're only worried about terrans trying to float
	if (enemyRace == Race::Terran)
	{
		//no base terrans might try to float barracks or other structures away
		if (enemyBases.empty())
		{
			return true;
		}
		else
		{
			//let's check the bases we know about
			bool landedBaseExists = false;
			for (auto base : enemyBases)
			{
				if (!base->is_flying)
				{
					return true;
				}
			}
			//if there's no landed bases then react
			if (landedBaseExists)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
	}
	else
	{
		return false;
	}
}

/*
adds a stargate and trains voidrays
*/
void ProductionManager::trainVoidray()
{
	//we don't wanna go stargate in PvT (unless we're just cleaning up the map with possible floating buildings) or on low economy
	if ((enemyRace == Race::Terran && !cleanUp()) || baseManager.getOurBases().size() < 3)
	{
		return;
	}
	//if we aren't already making one let's start a stargate
	else if (!structureExists(UNIT_TYPEID::PROTOSS_STARGATE) && !productionQueue.includes(ABILITY_ID::BUILD_STARGATE))
	{
		productionQueue.addItemHighPriority(ABILITY_ID::BUILD_STARGATE);
	}
	//if we've got a stargate then let's make a voidray
	else if (completedStructureExists(UNIT_TYPEID::PROTOSS_STARGATE) && !productionQueue.includes(ABILITY_ID::TRAIN_VOIDRAY) 
		&& blinkerBot.Observation()->GetFoodUsed() < 190)
	{
		productionQueue.addItemHighPriority(ABILITY_ID::TRAIN_VOIDRAY);
	}

}

/*
finds an idle warpgate
*/
const Unit *ProductionManager::getIdleWarpgate()
{
	int currentLoop = blinkerBot.Observation()->GetGameLoop();
	const Unit *idleGate = nullptr;
	for (auto gate : warpGates)
	{
		//std::cerr << currentLoop<< " last warped in " << UnitTypeToName(gate.lastTrainedType) << " at " << gate.lastWarpInLoop << std::endl;
		//compare how long it's been since we last trained a unit with the cooldown for the unit type we made

		if (currentLoop - gate.lastWarpInLoop >= UnitData::getWarpGateCoolDown(gate.lastTrainedType))
		{
			//blinkerBot.Debug()->DebugBoxOut(gate.warpgate->pos, Point3D(gate.warpgate->pos.x, gate.warpgate->pos.y, gate.warpgate->pos.y + 5));
			//blinkerBot.Debug()->SendDebug();
			return gate.warpgate;
		}
	}
	return nullptr;
}

/*
returns a base which doesn't have any pylons nearby, otherwise returns a nullptr
*/
const Unit *ProductionManager::baseNeedsNearbyPylon()
{
	const Unit *newBase = nullptr;

	//check each base to see if it has a nearby pylon
	for (auto base : baseManager.getOurBases())
	{
		bool nearbyPylon = false;
		for (auto pylon : pylons)
		{
			if (Distance2D(pylon->pos, base.getTownhall()->pos) <= NEARBYPYLONDIST + 1)
			{
				nearbyPylon = true;
			}
		}

		//if we can't find a nearby pylon, return the current base
		if (!nearbyPylon)
		{
			newBase = base.getTownhall();
			return newBase;
		}
	}
	//this should return nullptr;
	return newBase;
}

/*
receives a status from army manager telling us that the enemy army contains a significant amount of zerglings
*/
void ProductionManager::receiveMassLingSignal(bool signal)
{
	massLing = signal;
}

/*
starts upgrades at idle forges
*/
void ProductionManager::upgrade()
{
	for (auto structure : structures)
	{
		if (structure->unit_type == UNIT_TYPEID::PROTOSS_FORGE && structure->orders.empty())
		{
			//first prioritise attack upgrades
			if (!buildOrderManager.alreadyResearched(ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL3) && 
				!buildOrderManager.inProgress(ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS))
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS);
			}
			//if attack is already completed or upgrading, try armor
			else if (!buildOrderManager.alreadyResearched(ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL3) &&
				!buildOrderManager.inProgress(ABILITY_ID::RESEARCH_PROTOSSGROUNDARMOR))
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::RESEARCH_PROTOSSGROUNDARMOR);
			}
			//if we can't upgrade attack or armor, get shields
			else if (!buildOrderManager.alreadyResearched(ABILITY_ID::RESEARCH_PROTOSSSHIELDSLEVEL3) &&
				!buildOrderManager.inProgress(ABILITY_ID::RESEARCH_PROTOSSSHIELDS))
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::RESEARCH_PROTOSSSHIELDS);
			}
		}
	}
}

/*
trains extra sentries if we need them
*/
void ProductionManager::trainSentries()
{
	if (baseManager.getOurBases().empty())
	{
		return;
	}
	else if (armyStatus != Defend && 
		enemyRace != Terran &&
		!(beingRushed && shieldBatteries.size() < 2) &&
		!(!warpGates.empty() && warpGates.size() < 2) &&
		sentries.size() < assimilators.size() - 1 &&
		completedStructureExists(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) &&
		!productionQueue.includes(ABILITY_ID::TRAIN_SENTRY) &&
		!productionQueue.includes(ABILITY_ID::TRAINWARP_SENTRY))
	{
		if (warpGates.empty())
		{
			productionQueue.addItemHighPriority(ABILITY_ID::TRAIN_SENTRY);
		}
		else
		{
			productionQueue.addItemHighPriority(ABILITY_ID::TRAINWARP_SENTRY);
		}
	}
}

void ProductionManager::receiveLingSpeedSignal(bool signal)
{
	lingSpeed = signal;
}