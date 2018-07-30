#include "ProductionManager.h"
#include "Blinkerbot.h"

ProductionManager::ProductionManager(BlinkerBot & bot): blinkerBot(bot), forwardPylon(NULL), baseManager(bot), productionQueue(bot)
{
	productionQueue.initialiseQueue();
}


ProductionManager::~ProductionManager()
{
}

void ProductionManager::initialise()
{
	baseManager.initialise();
	rallyPoint = blinkerBot.Observation()->GetStartLocation();
	locateForwardPylonPoint();
}

void ProductionManager::onStep()
{
	BuildOrderItem nextBuildOrderItem = productionQueue.getNextItem();
	produce(nextBuildOrderItem);
	checkSupply();
	checkSaturation();

	const Unit *pylon = getClosestPylon((*blinkerBot.Observation()->GetGameInfo().enemy_start_locations.begin()));
	if (pylon)
	{
		rallyPoint = pylon->pos;
	}

	trainUnits();
	chronoBoost();
}

void ProductionManager::produce(BuildOrderItem nextBuildOrderItem)
{
	if (nextBuildOrderItem.item == ABILITY_ID::RESEARCH_BLINK || nextBuildOrderItem.item == ABILITY_ID::RESEARCH_WARPGATE)
	{
		for (auto structure : structures)
		{
			//deal with researches (currently only warpgate and blink are available)
			if ((structure->unit_type == UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL && nextBuildOrderItem.item == ABILITY_ID::RESEARCH_BLINK)
				|| (structure->unit_type == UNIT_TYPEID::PROTOSS_CYBERNETICSCORE && nextBuildOrderItem.item == ABILITY_ID::RESEARCH_WARPGATE))
			{
				//check if we already started it
				bool researchStarted = false;
				for (auto order : structure->orders)
				{
					if (order.ability_id == nextBuildOrderItem.item)
					{
						//if we've already started it then remove it from the to do list
						productionQueue.removeItem();
						researchStarted = true;
					}
				}
				//if we didn't start it then issue a command
				if (!researchStarted && (blinkerBot.Observation()->GetMinerals() >= blinkerBot.Observation()->GetUnitTypeData()[nextBuildOrderItem.item].mineral_cost)
					&& (blinkerBot.Observation()->GetVespene() >= blinkerBot.Observation()->GetUnitTypeData()[nextBuildOrderItem.item].vespene_cost))
				{
					blinkerBot.Actions()->UnitCommand(structure, nextBuildOrderItem.item);
				}
			}
		}
	}
	//deal with structures
	if (nextBuildOrderItem.item == ABILITY_ID::BUILD_NEXUS)
	{
		expand();
	}
	else if ((blinkerBot.Observation()->GetMinerals() >= blinkerBot.Observation()->GetUnitTypeData()[nextBuildOrderItem.item].mineral_cost)
		&& (blinkerBot.Observation()->GetVespene() >= blinkerBot.Observation()->GetUnitTypeData()[nextBuildOrderItem.item].vespene_cost))
	{
		buildStructure(nextBuildOrderItem.item, nextBuildOrderItem.location);
	}
}

void ProductionManager::trainUnits()
{
	for (auto structure : structures)
	{
		if (structure->unit_type == UNIT_TYPEID::PROTOSS_NEXUS && structure->orders.size() == 0 && getWorkerCount() < 45)
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
			if (blinkerBot.Observation()->GetVespene() > blinkerBot.Observation()->GetUnitTypeData()[UnitTypeID(UNIT_TYPEID::PROTOSS_STALKER)].vespene_cost)
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
			Point2D location = Point2D(rallyPoint.x + GetRandomScalar() * 15.0f, rallyPoint.y + GetRandomScalar() * 15.0f);
			if (blinkerBot.Observation()->GetVespene() > blinkerBot.Observation()->GetUnitTypeData()[UnitTypeID(UNIT_TYPEID::PROTOSS_STALKER)].vespene_cost)
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAINWARP_STALKER, location);
			}
			else
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAINWARP_ZEALOT, location);
			}
		}
		//make sure we have the right numbers of workers mining while we're here
		if (structure->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR)
		{
			checkGas(structure);
		}
	}
}

void ProductionManager::addWorker(const Unit *unit)
{
	workers.insert(unit);
	availableWorkers.insert(unit);
}

void ProductionManager::removeWorker(const Unit *unit)
{
	workers.erase(unit);
	availableWorkers.erase(unit);
}

void ProductionManager::addStructure(const Unit *unit)
{
	structures.insert(unit);
	if (unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
	{
		baseManager.addBase(unit);
	}
}

size_t ProductionManager::getWorkerCount()
{
	return workers.size();
}

std::set<std::pair<const Unit *, std::set<const Unit *>>> ProductionManager::getGasWorkers()
{
	return gasWorkers;
}

void ProductionManager::addGas(const Unit *gas)
{
	std::set<const Unit *> gasMiners;
	gasMiners.clear();
	int workersToAdd = 3;
	std::set<const Unit *>::const_iterator workerIterator = availableWorkers.begin();
	while (workersToAdd > 0)
	{
		gasMiners.insert(*workerIterator);
		availableWorkers.erase(*workerIterator);
		workerIterator = availableWorkers.begin();
		workersToAdd--;
	}
	gasWorkers.insert(std::make_pair(gas, gasMiners));
	checkGas(gas);
}

void ProductionManager::checkGas(const Unit *gas)
{
	for (auto pairs : gasWorkers)
	{
		if (pairs.first == gas)
		{
			for (auto worker : pairs.second)
			{
				if (worker->orders.empty())
				{
					blinkerBot.Actions()->UnitCommand(worker, ABILITY_ID::SMART, gas);
				}
				else
				{
					for (auto order : worker->orders)
					{
						if ((order.ability_id != ABILITY_ID::HARVEST_RETURN) && (order.target_pos != gas->pos))
						{
							blinkerBot.Actions()->UnitCommand(worker, ABILITY_ID::SMART, gas);
						}
					}
				}
			}
		}
	}
}

const Unit *ProductionManager::getBuilder()
{
	if (availableWorkers.size() > 0)
	{
		std::set<const Unit *>::const_iterator builder = availableWorkers.begin();
		for (auto gas : gasWorkers)
		{
			for (auto worker : gas.second)
			{
				if (*builder == worker)
				{
					availableWorkers.erase(worker);
					builder = availableWorkers.begin();

				}
			}
		}
		return *builder;
	}
	else
	{
		std::cerr << "No workers Available" << std::endl;
		return nullptr;
	}
}

void ProductionManager::buildStructure(AbilityID structureToBuild, Location location)
{
	const Unit *builder = getBuilder();
	if (structureToBuild == ABILITY_ID::BUILD_ASSIMILATOR)
	{
		blinkerBot.Actions()->UnitCommand(builder, structureToBuild, baseManager.getNextAvailableGas());
	}
	else
	{
		Point2D buildLocation;
		switch (location)
		{
		case Proxy:
			buildLocation = Point2D(forwardPylonPoint.x + GetRandomScalar() * 15.0f, forwardPylonPoint.y + GetRandomScalar() * 15.0f);
			blinkerBot.Actions()->UnitCommand(builder, structureToBuild, buildLocation);
			break;
		case Main:
			buildLocation = Point2D(blinkerBot.Observation()->GetStartLocation().x + GetRandomScalar() * 15.0f,
				blinkerBot.Observation()->GetStartLocation().y + GetRandomScalar() * 15.0f);
			blinkerBot.Actions()->UnitCommand(builder, structureToBuild, buildLocation);
			break;
		case Nowhere:
			productionQueue.removeItem();
			productionQueue.generateMoreItems(generateBuildOrderGoal());
			break;
		case Natural:
		case Third:
		default:
			buildLocation = Point2D(builder->pos.x + GetRandomScalar() * 15.0f, builder->pos.y + GetRandomScalar() * 15.0f);
			blinkerBot.Actions()->UnitCommand(builder, structureToBuild, buildLocation);
			break;
		}
	}
}

void ProductionManager::buildStructure(AbilityID structureToBuild, Point2D target)
{
	const Unit *builder = getBuilder();
	blinkerBot.Actions()->UnitCommand(builder, structureToBuild, target);
}

void ProductionManager::buildStructure(const Unit *builder, AbilityID structureToBuild, Point2D target)
{
	blinkerBot.Actions()->UnitCommand(builder, structureToBuild, target);
}


void ProductionManager::returnToMining(const Unit *unit)
{	
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
}

void ProductionManager::addNewUnit(const Unit *unit)
{
	if (blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].ability_id == productionQueue.getNextItem().item)
	{
		productionQueue.removeItem();
	}
	if (UnitData::isStructure(unit))
	{
		addStructure(unit);
	}
	if (unit->unit_type == UNIT_TYPEID::PROTOSS_PYLON)
	{
		pylons.insert(unit);
	}
	if (unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE)
	{
		addWorker(unit);
	}
	if (unit->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR)
	{
		baseManager.addGas(unit);
	}
	if (unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS && blinkerBot.Observation()->GetGameLoop() > 1)
	{
		baseManager.removeNextBaseLocation();
	}
}

void ProductionManager::onUnitDestroyed(const Unit *unit)
{
	if (unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE)
	{
		removeWorker(unit);
	}
	else
	{	
		for (std::set<const Unit *>::iterator structure = structures.begin(); structure != structures.end();)
		{
			if ((*structure) == unit)
			{
				structures.erase(*structure++);

				//if we lost an expansion, make this point available to be expanded on again
				if (unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
				{
					baseManager.removeBase(unit);
					baseManager.addAvailableBaseLocation(unit);
				}
				//if it was a gas then we need to update baseManager too
				else if (unit->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR)
				{
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

void ProductionManager::receiveAttackSignal(bool attack)
{
	//if (attack && forwardPylon == NULL)
	if (attack && pylons.size() > 0 && Distance2D(getClosestPylon(forwardPylonPoint)->pos, forwardPylonPoint) > 50.0f 
		&& productionQueue.getNextItem().location != Proxy)
	{
		//std::cerr << "closest pylon is " << Distance2D(getClosestPylon(forwardPylonPoint)->pos, forwardPylonPoint) << " away" << std::endl;
		//std::cerr << "adding proxy pylon to build queue" << std::endl;
productionQueue.addItemHighPriority(ABILITY_ID::BUILD_PYLON, Proxy);
	}
	else if (pylons.size() > 0 && Distance2D(getClosestPylon(forwardPylonPoint)->pos, forwardPylonPoint) < 50.0f)
	{
		//std::cerr << "we've already got a forward pylon" << std::endl;
	}
}

void ProductionManager::locateForwardPylonPoint()
{
	Point2D centre = Point2D(blinkerBot.Observation()->GetGameInfo().width / 2.0f, blinkerBot.Observation()->GetGameInfo().height / 2.0f);
	forwardPylonPoint = Point2D(centre.x + GetRandomScalar() * 15.0f, centre.y + GetRandomScalar() * 15.0f);
	//forwardPylonPoint = centre;
}

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
		return NULL;
	}
}

Point2D ProductionManager::getRallyPoint()
{
	return rallyPoint;
}

void ProductionManager::checkSupply()
{
	float supplyCapacity = 0;
	float supplyUsed = 0;
	bool pylonAlreadyInProduction = false;

	//calculate our current supply capacity
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
	if (supplyCapacity - supplyUsed < 2 && productionQueue.getNextItem().item != ABILITY_ID::BUILD_PYLON && !pylonAlreadyInProduction)
	{
		std::cerr << "we're blocked so making a new pylon" << std::endl;
		productionQueue.addItemHighPriority(ABILITY_ID::BUILD_PYLON, Main);
	}
}





void ProductionManager::expand()
{

	Point2D nextBase = baseManager.getNextBaseLocation();

	if (blinkerBot.Query()->Placement(ABILITY_ID::BUILD_NEXUS, nextBase, getBuilder()))
	{
		buildStructure(ABILITY_ID::BUILD_NEXUS, nextBase);
	}
	else
	{
		buildStructure(ABILITY_ID::BUILD_NEXUS, Point2D(nextBase.x + GetRandomScalar(), nextBase.y + GetRandomScalar()));
	}
}

void ProductionManager::addEnemyBase(const Unit *unit)
{
	enemyBases.insert(unit);
}
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

std::set<std::pair<AbilityID, int>> ProductionManager::generateBuildOrderGoal()
{
	std::set<std::pair<AbilityID, int>> buildOrderGoal;
	int currentBases = int(baseManager.getOurBases().size());
	int currentProductionFacilities = 0;

	for (auto structure : structures)
	{
		if (structure->unit_type == UNIT_TYPEID::PROTOSS_GATEWAY || structure->unit_type == UNIT_TYPEID::PROTOSS_WARPGATE)
		{
			currentProductionFacilities++;
		}
	}

	int extraProductionFacilities = (currentBases * 3) - currentProductionFacilities;
	buildOrderGoal.insert(std::make_pair(ABILITY_ID::BUILD_GATEWAY, extraProductionFacilities));

	if (currentBases <= enemyBases.size() && extraProductionFacilities == 0)
	{
		buildOrderGoal.insert(std::make_pair(ABILITY_ID::BUILD_NEXUS, 1));
	}

	std::cerr << "current bases: " << currentBases << std::endl;
	std::cerr << "current gateways: " << currentProductionFacilities << std::endl;
	std::cerr << "goal state: " << currentBases << " bases and " << currentProductionFacilities + extraProductionFacilities << " gateways" << std::endl;
	std::cerr << "adding to production queue:" << std::endl;
	for (auto item : buildOrderGoal)
	{
		for (int i = 0; i != item.second; i++)
		{
			std::cerr << AbilityTypeToName(item.first) << std::endl;
		}
	}

	if (buildOrderGoal.empty())
	{
		buildOrderGoal.insert(std::make_pair(ABILITY_ID::BUILD_PYLON, 1));
	}

	return buildOrderGoal;
}

void ProductionManager::checkSaturation()
{
	for (auto base : baseManager.getOurBases())
	{
		for (auto structure : structures)
		{
			if (Distance2D(base.getBuildLocation(), structure->pos) < 4 && structure->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
			{
				if (structure->assigned_harvesters > structure->ideal_harvesters)
				{
					//std::cerr << "we've got too many workers at a base" << std::endl;
					transferWorkers(structure->assigned_harvesters - structure->ideal_harvesters, base);
				}
			}
		}
	}
}

//takes an oversaturated base and the amount of workers we want to move and tries to find another place for them to go
void ProductionManager::transferWorkers(int numOfWorkers, Base overSaturatedBase)
{
	for (auto base : baseManager.getOurBases())
	{
		//make sure we're looking at a different base location
		if (base.getBuildLocation() != overSaturatedBase.getBuildLocation())
		{
			for (auto structure : structures)
			{
				//find the nexus for this base
				if (Distance2D(base.getBuildLocation(), structure->pos) < 4 && structure->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
				{
					//std::cerr << "we found another nexus" << std::endl;
					//calculate how many workers we can take on
					int freeSpace = structure->ideal_harvesters - structure->assigned_harvesters;
					std::set<const Unit *>::iterator worker = availableWorkers.begin();
					while (freeSpace > 0 && numOfWorkers > 0 && worker != availableWorkers.end())
					{
						//std::cerr << "found an undersaturated nexus" << std::endl;
						if (Distance2D((*worker)->pos, overSaturatedBase.getBuildLocation()) < 5)
						{
							//std::cerr << "transferring a worker" << std::endl;
							
							blinkerBot.Actions()->UnitCommand((*worker), ABILITY_ID::MOVE, base.getBuildLocation());
							//blinkerBot.Actions()->UnitCommand((*worker), ABILITY_ID::SMART, (*base.getMinerals().begin()));
							freeSpace--;
							numOfWorkers--;
						}
						worker++;
					}
				}
			}
		}
	}
}

//find our nexuses and use chronoboost if they have enough energy
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

				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::EFFECT_CHRONOBOOST, target);
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::EFFECT_MASSRECALL, structure->pos);
			}
		}
	}
	/*
	for (auto item : getCurrentlyInProduction())
	{
		std::cerr << AbilityTypeToName(item.first) << " at " << UnitTypeToName(item.second->unit_type) << std::endl;
	}*/
}

//return a set of pairs representing all the things that are currently in production at our structures along with a pointer to the corresponding structure
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

//take the set of things that are currently in production, and return a pointer to the structure which is producing the most important one
const Unit * ProductionManager::getHighestPriorityInProduction(std::set<std::pair<AbilityID, const Unit *>> inProduction)
{
	const Unit *highest = nullptr;
	if (inProduction.size() > 0)
	{
		highest = (*inProduction.begin()).second;
	}

	return highest;
}