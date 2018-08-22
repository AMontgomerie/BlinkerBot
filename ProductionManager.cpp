#include "ProductionManager.h"
#include "Blinkerbot.h"

ProductionManager::ProductionManager(BlinkerBot & bot): 
	blinkerBot(bot), baseManager(bot), productionQueue(bot), workerManager(bot), buildOrderManager(bot), 
	forwardPylon(nullptr), nextPylonLocation(Point2D(-1.0,-1.0)), attacking(false), enemyHasCloak(false), lastProductionFrame(0)
{
	productionQueue.initialiseQueue();
}


ProductionManager::~ProductionManager()
{
}

void ProductionManager::initialise()
{
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
	baseManager.initialise();
	workerManager.initialise();
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

	if (blinkerBot.Observation()->GetGameLoop() - lastProductionFrame > DEADLOCK
		&& uint32_t(blinkerBot.Observation()->GetMinerals()) > DEADLOCK)
	{
		std::cerr << "attempting to break production deadlock" << std::endl;
		breakDeadlock();
	}

	//if the first pylon location hasn't been set yet, then let's set it
	if (nextPylonLocation == Point2D(-1, -1))
	{
		setNextPylonLocation();
	}

	//make sure that we aren't supply blocked
	checkSupply();
	//deal with the next item in the production queue
	produce(productionQueue.getNextItem());

	//update workers (called frequently because of scouting worker micro)
	workerManager.update();

	//the following don't need to be called so frequently
	if (blinkerBot.Observation()->GetGameLoop() % 30 == 0)
	{
		chronoBoost();

		//let's stop production briefly to make sure the next base actually gets started
		if (!(productionQueue.getNextItem().item == ABILITY_ID::BUILD_NEXUS && 
			blinkerBot.Observation()->GetGameLoop() - lastProductionFrame < DEADLOCK))
		{
			trainUnits();
		}

		//if worker manager sends a warning that a base is mining out, build order manager needs to be informed
		buildOrderManager.receiveMiningOutSignal(workerManager.miningOut());
	}

	/*
	//timer stuff
	duration = (std::clock() - start) / (double)CLOCKS_PER_SEC * 1000;
	if (duration > 20)
	{
		std::cout << "ProductionManager duration: " << duration << '\n';
	}
	//timer stuff
	*/
}

/*
determines an appropriate rally point
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

void ProductionManager::setRallyPoint(Point2D point)
{
	rallyPoint = point;
}

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
	else if (nextBuildOrderItem.item == ABILITY_ID::BUILD_NEXUS)
	{
		expand();
	}
	//deal with structures
	else
	{
		build(nextBuildOrderItem);
	}
}

//trains units automatically (independent of productionQueue)
void ProductionManager::trainUnits()
{
	bool haveCyberneticscore = false;
	for (auto structure : structures)
	{
		if (structure->unit_type == UNIT_TYPEID::PROTOSS_CYBERNETICSCORE && structure->build_progress == 1.0)
		{
			haveCyberneticscore = true;
		}
	}
	for (auto structure : structures)
	{
		if (structure->unit_type == UNIT_TYPEID::PROTOSS_NEXUS && structure->orders.size() == 0 
			&& workerManager.getWorkerCount() <= (calculateMiningBases() * 22))
		{
			blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAIN_PROBE);
		}
		else if (structure->unit_type == UNIT_TYPEID::PROTOSS_GATEWAY && structure->orders.size() == 0)
		{
			//if we have an idle gateway, try to make it into a warpgate
			if (structure->unit_type == UNIT_TYPEID::PROTOSS_GATEWAY)
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::MORPH_WARPGATE);
			}
			//otherwise train some units
			if (haveCyberneticscore &&
				blinkerBot.Observation()->GetVespene() > blinkerBot.Observation()->GetUnitTypeData()[UnitTypeID(UNIT_TYPEID::PROTOSS_STALKER)].vespene_cost)
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAIN_STALKER);
			}
			else
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAIN_ZEALOT);
			}
		}
		else if (structure->unit_type == UNIT_TYPEID::PROTOSS_WARPGATE)
		{
			Point2D location = Point2D(rallyPoint.x + GetRandomScalar() * 7.0f, rallyPoint.y + GetRandomScalar() * 7.0f);
			if (haveCyberneticscore &&
				blinkerBot.Observation()->GetVespene() > blinkerBot.Observation()->GetUnitTypeData()[UnitTypeID(UNIT_TYPEID::PROTOSS_STALKER)].vespene_cost)
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAINWARP_STALKER, location);
			}
			else
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAINWARP_ZEALOT, location);
			}
		}
		else if (structure->unit_type == UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY)
		{
			blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAIN_COLOSSUS);
		}
	}
}

/*
takes a given BuildOrderItem and attempts to build a corresponding structure. Queues up required tech structures if necessary
*/
void ProductionManager::build(BuildOrderItem item)
{
	//if we're already at supply capacity, we don't want to make any more pylons
	if (item.item == ABILITY_ID::BUILD_PYLON && calculateSupplyCapacity() >= 200)
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
	else
	{
		//if we have the tech, then let's check if we have the necessary resources
		if ((blinkerBot.Observation()->GetMinerals() >= blinkerBot.Observation()->GetUnitTypeData()[UnitData::getUnitTypeID(item.item)].mineral_cost)
			&& (blinkerBot.Observation()->GetVespene() >= blinkerBot.Observation()->GetUnitTypeData()[UnitData::getUnitTypeID(item.item)].vespene_cost))
		{
			buildStructure(item.item);
		}
	}
}

//trains a unit from the production queue, or adds extra items to the queue in the event that we don't have the necessary tech
void ProductionManager::train(BuildOrderItem item)
{
	bool isStarted = false;
	bool isTrainable = false;
	const Unit *productionFacility = nullptr;
	//UnitTypeID requiredStructure = blinkerBot.Observation()->GetUnitTypeData()[UnitData::getUnitTypeID(item.item)].tech_requirement;

	UnitTypeID requiredStructure = UnitData::requiredStructure(item.item);

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
		if ((blinkerBot.Observation()->GetMinerals() >= blinkerBot.Observation()->GetUnitTypeData()[UnitData::getUnitTypeID(item.item)].mineral_cost)
			&& (blinkerBot.Observation()->GetVespene() >= blinkerBot.Observation()->GetUnitTypeData()[UnitData::getUnitTypeID(item.item)].vespene_cost))
		{
			blinkerBot.Actions()->UnitCommand(productionFacility, item.item);
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
	else if (!isStarted)
	{
		//check if we have the necessary resources (these techs don't seem to be in upgrade data)
		//if ((blinkerBot.Observation()->GetMinerals() >= blinkerBot.Observation()->GetUpgradeData()[item.item].mineral_cost)
		//	&& (blinkerBot.Observation()->GetVespene() >= blinkerBot.Observation()->GetUpgradeData()[item.item].vespene_cost))
		//{
			blinkerBot.Actions()->UnitCommand(researchStructure, item.item);
		//}
	}
}

void ProductionManager::addStructure(const Unit *unit)
{
	structures.insert(unit);
	if (UnitData::isOurs(unit) && unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
	{
		baseManager.addBase(unit);
		workerManager.addBase(unit);
	}
}

void ProductionManager::buildStructure(AbilityID structureToBuild)
{
	const Unit *builder = workerManager.getBuilder();

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
			blinkerBot.Actions()->UnitCommand(builder, structureToBuild, 
				Point2D(nextPylonLocation.x + GetRandomScalar() * 2, nextPylonLocation.y + GetRandomScalar() * 2));
		}

		//we want to make sure we have a cannon at each base for detection purposes
		else if (structureToBuild == ABILITY_ID::BUILD_PHOTONCANNON)
		{
			Point2D buildLocation;
			
			if (baseManager.getOurBases().empty())
			{
				buildLocation = Point2D(getLeastArtosisPylon()->pos.x + GetRandomScalar() * 6.5f, getLeastArtosisPylon()->pos.y + GetRandomScalar() * 6.5f);
			}
			else
			{
				for (auto base : baseManager.getOurBases())
				{
					//check each base to see if a base has a cannon
					bool hasCannon = false;
					for (auto structure : structures)
					{
						if (Distance2D(base.getBuildLocation(), structure->pos) < 10 && structure->unit_type == UNIT_TYPEID::PROTOSS_PHOTONCANNON)
						{
							hasCannon = true;
						}
					}
					//if we find a base with no cannon, let's get that location
					if (!hasCannon)
					{
						buildLocation = base.getBuildLocation();
					}
				}
				//find the closest pylon to that base, and build the cannon there
				const Unit *targetPylon = getClosestPylon(buildLocation);
				if (targetPylon)
				{
					buildLocation = Point2D(targetPylon->pos.x + GetRandomScalar() * 6.5f, targetPylon->pos.y + GetRandomScalar() * 6.5f);
				}
			}
			blinkerBot.Actions()->UnitCommand(builder, structureToBuild, buildLocation);
		}
		else
		{

			//find a pylon with some space and choose a random location within its power radius
			Point2D buildLocation;
			const Unit *targetPylon = getLeastArtosisPylon();
			buildLocation = Point2D(targetPylon->pos.x + GetRandomScalar() * 6.5f, targetPylon->pos.y + GetRandomScalar() * 6.5f);
			blinkerBot.Actions()->UnitCommand(builder, structureToBuild, buildLocation);
		}
	}
}

//returns the pylon powering the least number of structures
const Unit *ProductionManager::getLeastArtosisPylon()
{
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

void ProductionManager::setNextPylonLocation()
{
	//if we need a forward pylon then build the pylon there
	if (forwardPylon == nullptr && attacking)
	{
		nextPylonLocation = forwardPylonPoint;
	}
	//otherwise find an appropriate location
	else
	{
		Point2D buildLocation;
		//find the location of our most recent base
		if (baseManager.getOurBases().empty())
		{
			buildLocation = blinkerBot.Observation()->GetStartLocation();
		}
		else
		{
			buildLocation = baseManager.getOurBases().back().getBuildLocation();
		}

		//get the grid of buildable locations for the area around that base
		std::vector<Point2D> buildGrid = getBuildGrid(buildLocation);

		//select a random location from the available ones
		buildLocation = GetRandomEntry(buildGrid);

		nextPylonLocation = buildLocation;
	}
	/*
	Point3D point = Point3D(nextPylonLocation.x, nextPylonLocation.y, blinkerBot.Observation()->GetStartLocation().z + 1);
	blinkerBot.Debug()->DebugBoxOut(point, Point3D(point.x + 1, point.y + 1, point.z));
	blinkerBot.Debug()->SendDebug();
	std::cerr << nextPylonLocation.x << ":" << nextPylonLocation.y << std::endl;
	*/
}

/*
build a structure at a specified point on the map
*/
void ProductionManager::buildStructure(AbilityID structureToBuild, Point2D target)
{
	const Unit *builder = workerManager.getBuilder();
	blinkerBot.Actions()->UnitCommand(builder, structureToBuild, target);
}

/*
build a structure at a specified point on the map using the supplied unit
*/
void ProductionManager::buildStructure(const Unit *builder, AbilityID structureToBuild, Point2D target)
{
	blinkerBot.Actions()->UnitCommand(builder, structureToBuild, target);
}

/*
updates the relevant sets and vectors when a new unit is created
*/
void ProductionManager::addNewUnit(const Unit *unit)
{
	if (blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].ability_id == productionQueue.getNextItem().item)
	{
		productionQueue.removeItem();
		lastProductionFrame = blinkerBot.Observation()->GetGameLoop();
	}
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
	if (UnitData::isOurs(unit) && unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE)
	{
		workerManager.addWorker(unit);
	}
	if (UnitData::isOurs(unit) && unit->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR)
	{
		workerManager.addGas(unit);
		baseManager.addGas(unit);
	}
	if (unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS && blinkerBot.Observation()->GetGameLoop() > 1)
	{
		
		baseManager.removeNextBaseLocation();
	}
}

/*
updates the relevant sets and vectors when a unit dies
*/
void ProductionManager::onUnitDestroyed(const Unit *unit)
{
	if (unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE)
	{
		workerManager.removeWorker(unit);
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
lets the production manager know whether we are attacking or not. The value is supplied by the army manager.
*/
void ProductionManager::receiveAttackSignal(bool attack)
{
	attacking = attack;
	/*
	//if (attack && forwardPylon == NULL)
	if (attack && pylons.size() > 0 && Distance2D(getClosestPylon(forwardPylonPoint)->pos, forwardPylonPoint) > 50.0f)
	{
		productionQueue.addItemHighPriority(ABILITY_ID::BUILD_PYLON);
	}
	else if (pylons.size() > 0 && Distance2D(getClosestPylon(forwardPylonPoint)->pos, forwardPylonPoint) < 50.0f)
	{
		//std::cerr << "we've already got a forward pylon" << std::endl;
	}
	*/
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
		forwardPylonPoint = proxyBase.getBuildLocation();
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
		if (UnitData::isOurs(unit) && unit->is_alive)
		{
			supplyUsed += blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].food_required;
		}
	}
	//check we don't already have a pylon in production
	for (auto pylon : pylons)
	{
		if (pylon->build_progress < 1.0)
		{
			pylonAlreadyInProduction = true;
		}
	}
	//if we're nearly blocked, the next item in the queue is not a pylon, and we don't have one already building, then make a new one
	if (supplyCapacity - supplyUsed < 4 && productionQueue.getNextItem().item != ABILITY_ID::BUILD_PYLON && !pylonAlreadyInProduction)
	{
		productionQueue.addItemHighPriority(ABILITY_ID::BUILD_PYLON);
		//add an extra pylon for each base we have
		for (auto base : baseManager.getOurBases())
		{
			productionQueue.addItemHighPriority(ABILITY_ID::BUILD_PYLON);
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
				//std::cerr << UnitTypeToName(structure->unit_type) << " is boostin " << UnitTypeToName(target->unit_type) << std::endl;

				//3755 = new chronoboost. EFFECT_CHRONOBOOST is the old pre-LOTV one.
				blinkerBot.Actions()->UnitCommand(structure, AbilityID(3755), target);
			}
		}
	}
	/*
	for (auto item : getCurrentlyInProduction())
	{
		std::cerr << AbilityTypeToName(item.first) << " at " << UnitTypeToName(item.second->unit_type) << std::endl;
	}*/
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
	switch (ability)
	{
	case ABILITY_ID::INVALID:
		return 100;
		break;
	case ABILITY_ID::RESEARCH_WARPGATE:
		return 1;
		break;
	case ABILITY_ID::RESEARCH_BLINK:
	case ABILITY_ID::RESEARCH_EXTENDEDTHERMALLANCE:
	case ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS:
		return 2;
		break;
	case ABILITY_ID::TRAIN_STALKER:
		return 3;
		break;
	default:
		return 4;
		break;
	}
}

/*
if army manager sends us a signal alerting us of undetected cloak, let's make some detection
*/
void ProductionManager::receiveCloakSignal(bool signal)
{
	buildOrderManager.receiveCloakSignal(signal);

	if (signal)
	{
		enemyHasCloak = true;
		//check if we have the ability to produce detectors
		bool detectionProducable = false;
		for (auto structure : structures)
		{
			if (structure->unit_type == UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY) //||
				//structure->unit_type == UNIT_TYPEID::PROTOSS_FORGE)
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
			if (item.first == ABILITY_ID::BUILD_ROBOTICSFACILITY)
			{
				alreadyConstructing = true;
			}
		}

		//build or train if necessary
		if (!detectionProducable && !alreadyConstructing)
		{
			if (!productionQueue.includes(ABILITY_ID::BUILD_ROBOTICSFACILITY))
			{
				productionQueue.addItemHighPriority(ABILITY_ID::BUILD_ROBOTICSFACILITY);
			}
		}
		else if (detectionProducable && !alreadyTraining)
		{
			if (!productionQueue.includes(ABILITY_ID::TRAIN_OBSERVER))
			{
				productionQueue.addItemHighPriority(ABILITY_ID::TRAIN_OBSERVER);
			}
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

/*
prints a build grid of the whole map (causes extreme FPS drop)
*/
void ProductionManager::printDebug()
{
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
		if (*point == Point2D(0, 0))
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
		setNextPylonLocation();
	}
	//let's delete whatever was in our current queue, and generate a new one
	productionQueue.clearQueue();
	productionQueue.generateMoreItems(buildOrderManager.generateGoal());
	lastProductionFrame = blinkerBot.Observation()->GetGameLoop();
}

/*
tells a worker to mine minerals (called by BlinkerBot::OnUnitIdle)
REDUNDANT: now handled by WorkerManager
*/
void ProductionManager::returnToMining(const Unit *unit)
{
	/*
	//first let's check if there's any visible minerals nearby
	bool nextToMineral = false;
	for (auto mineral : blinkerBot.Observation()->GetUnits())
	{
		//if we find some nearby visible minerals, let's right click 'em
		if (mineral->mineral_contents > 0 && Distance2D(mineral->pos, unit->pos) < 10 && mineral->display_type == Unit::DisplayType::Visible)
		{
			nextToMineral = true;
			blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral);
		}
	}
	//if there's no visible minerals nearby, issue a move command towards the minerals at the closest base
	//we issue move rather than smart here because smart can't right click on units with a display_type of snapshot (which might be the case here)
	if (!nextToMineral)
	{
		//find the closest base
		Base closestBase = *baseManager.getOurBases().begin();
		for (auto base : baseManager.getOurBases())
		{
			if (Distance2D(unit->pos, base.getBuildLocation()) < Distance2D(unit->pos, closestBase.getBuildLocation()))
			{
				closestBase = base;
			}
		}
		const Unit * mineral = *closestBase.getMinerals().begin();
		//move towards the minerals
		blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::MOVE, mineral);
	}
	*/
}
