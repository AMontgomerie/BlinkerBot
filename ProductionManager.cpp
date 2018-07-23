#include "ProductionManager.h"
#include "Blinkerbot.h"

ProductionManager::ProductionManager(BlinkerBot & bot): blinkerBot(bot), forwardPylon(NULL)
{
	productionQueue.initialiseQueue();
}


ProductionManager::~ProductionManager()
{
}

void ProductionManager::initialise()
{
	findGeysers();
	rallyPoint = blinkerBot.Observation()->GetStartLocation();
	locateForwardPylonPoint();
}

void ProductionManager::onStep()
{
	BuildOrderItem nextBuildOrderItem = productionQueue.getNextItem();
	produce(nextBuildOrderItem);
	if(blinkerBot.Observation()->GetFoodCap() - blinkerBot.Observation()->GetFoodUsed() < 5)
	{
		productionQueue.generateMoreItems();
	}
	const Unit *pylon = getClosestPylon((*blinkerBot.Observation()->GetGameInfo().enemy_start_locations.begin()));
	if (pylon)
	{
		rallyPoint = pylon->pos;
	}
	trainUnits();
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
	if ((blinkerBot.Observation()->GetMinerals() >= blinkerBot.Observation()->GetUnitTypeData()[nextBuildOrderItem.item].mineral_cost)
		&& (blinkerBot.Observation()->GetVespene() >= blinkerBot.Observation()->GetUnitTypeData()[nextBuildOrderItem.item].vespene_cost))
	{
		buildStructure(nextBuildOrderItem.item, nextBuildOrderItem.location);
	}
}

void ProductionManager::trainUnits()
{
	for (auto structure : structures)
	{
		if (structure->unit_type == UNIT_TYPEID::PROTOSS_NEXUS && structure->orders.size() == 0 && getWorkerCount() < 23)
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
		//currently only works in the main base
		blinkerBot.Actions()->UnitCommand(builder, structureToBuild, getClosestGas(blinkerBot.Observation()->GetStartLocation()));
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


const Unit *ProductionManager::getClosestGas(const Unit *unit)
{
	//std::cerr << "finding closest gas by unit" << std::endl;
	const Unit *closestGas = nullptr;

	//std::cerr << "searching among " << availableGeysers.size() << " gases" << std::endl;
	for (auto gas : availableGeysers)
	{
		if (closestGas == nullptr)
		{
			closestGas = gas;
		}
		else if (Distance2D(unit->pos, gas->pos) <= Distance2D(unit->pos, closestGas->pos))
		{
			closestGas = gas;
		}
	}
	if (closestGas == nullptr)
	{
		std::cerr << "no gas found" << std::endl;
	}
	return closestGas;
}

const Unit *ProductionManager::getClosestGas(Point3D point)
{
	//std::cerr << "finding closest gas by point" << std::endl;
	const Unit *closestGas = nullptr;

	//std::cerr << "searching among " << availableGeysers.size() << " gases" << std::endl;
	for (auto gas : availableGeysers)
	{
		if (closestGas == nullptr)
		{
			closestGas = gas;
		}
		else if (Distance2D(point, gas->pos) <= Distance2D(point, closestGas->pos))
		{
			closestGas = gas;
		}
	}
	if (closestGas == nullptr)
	{
		std::cerr << "no gas found" << std::endl;
	}
	return closestGas;
}


void ProductionManager::returnToMining(const Unit *unit)
{
	for (auto minerals : blinkerBot.Observation()->GetUnits())
	{
		if (minerals->mineral_contents > 0)
		{
			addWorker(unit);
			blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::SMART, minerals);
		}
	}
}

void ProductionManager::addNewUnit(const Unit *unit)
{
	if (blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].ability_id == productionQueue.getNextItem().item)
	{/*
		if (unit->pos.x == forwardPylonPoint.x && unit->pos.y == forwardPylonPoint.y)
		{
			std::cerr << "forward pylon in production" << std::endl;
			forwardPylon = unit;
			//rallyPoint = forwardPylonPoint;
		}*/
		productionQueue.removeItem();
		if (unit->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR)
		{
			for (std::set<const Unit *>::iterator geyser = availableGeysers.begin(); geyser != availableGeysers.end();)
			{
				if (((*geyser)->pos.x == unit->pos.x) && ((*geyser)->pos.y == unit->pos.y))
				{
					availableGeysers.erase(geyser++);
				}
				else
				{
					++geyser;
				}
			}
		}
		if (unit->unit_type == UNIT_TYPEID::PROTOSS_PYLON)
		{
			pylons.insert(unit);
		}
	}
	if (unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE)
	{
		addWorker(unit);
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

void ProductionManager::findGeysers()
{
	for (auto unit : blinkerBot.Observation()->GetUnits())
	{
		if ((unit->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER) ||
			(unit->unit_type == UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER) ||
			(unit->unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER) ||
			(unit->unit_type == UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER) ||
			(unit->unit_type == UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER) ||
			(unit->unit_type == UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER))
		{
			availableGeysers.insert(unit);
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