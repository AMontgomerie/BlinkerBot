#include "ProductionManager.h"
#include "Blinkerbot.h"

ProductionManager::ProductionManager(BlinkerBot & bot): blinkerBot(bot) //, issuedBuildCommand(false)
{
	productionQueue.initialiseQueue();
}


ProductionManager::~ProductionManager()
{
}

void ProductionManager::initialise()
{
	findGeysers();
}

void ProductionManager::onStep()
{
	AbilityID nextBuildOrderItem = productionQueue.getNextItem();
	if (nextBuildOrderItem)
	{
		produce(nextBuildOrderItem);
	}
	else
	{
		productionQueue.generateMoreItems();
	}
	trainUnits();
}

void ProductionManager::produce(AbilityID nextBuildOrderItem)
{
	if (nextBuildOrderItem == ABILITY_ID::RESEARCH_BLINK)
	{
		for (auto structure : structures)
		{
			if (structure->unit_type == UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL)
			{
				bool researchStarted = false;
				for (auto order : structure->orders)
				{
					if (order.ability_id == nextBuildOrderItem)
					{
						productionQueue.removeItem();
						researchStarted = true;
					}
				}
				if (!researchStarted && (blinkerBot.Observation()->GetMinerals() >= blinkerBot.Observation()->GetUnitTypeData()[nextBuildOrderItem].mineral_cost)
					&& (blinkerBot.Observation()->GetVespene() >= blinkerBot.Observation()->GetUnitTypeData()[nextBuildOrderItem].vespene_cost))
				{
					blinkerBot.Actions()->UnitCommand(structure, nextBuildOrderItem);
				}
			}
		}
	}
	if ((blinkerBot.Observation()->GetMinerals() >= blinkerBot.Observation()->GetUnitTypeData()[nextBuildOrderItem].mineral_cost)
		&& (blinkerBot.Observation()->GetVespene() >= blinkerBot.Observation()->GetUnitTypeData()[nextBuildOrderItem].vespene_cost))
	{
		buildStructure(nextBuildOrderItem);
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
			if (blinkerBot.Observation()->GetVespene() > blinkerBot.Observation()->GetUnitTypeData()[UnitTypeID(UNIT_TYPEID::PROTOSS_STALKER)].vespene_cost)
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAIN_STALKER);
			}
			else
			{
				blinkerBot.Actions()->UnitCommand(structure, ABILITY_ID::TRAIN_ZEALOT);
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

void ProductionManager::buildStructure(AbilityID structureToBuild)
{
	const Unit *builder = getBuilder();
	if (structureToBuild == ABILITY_ID::BUILD_ASSIMILATOR)
	{
		std::cerr << "trying to build an assimilator" << std::endl;
		blinkerBot.Actions()->UnitCommand(builder, structureToBuild, getClosestGas(blinkerBot.Observation()->GetStartLocation()));
	}
	else
	{
		Point2D buildLocation = Point2D(builder->pos.x + GetRandomScalar() * 15.0f, builder->pos.y + GetRandomScalar() * 15.0f);
		blinkerBot.Actions()->UnitCommand(builder, structureToBuild, buildLocation);
	}
}

const Unit *ProductionManager::getClosestGas(const Unit *unit)
{
	std::cerr << "finding closest gas by point" << std::endl;
	const Unit *closestGas = nullptr;

	std::cerr << "searching among " << availableGeysers.size() << " gases" << std::endl;
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
	std::cerr << "finding closest gas by point" << std::endl;
	const Unit *closestGas = nullptr;

	std::cerr << "searching among " << availableGeysers.size() << " gases" << std::endl;
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
			blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::SMART, minerals);
		}
	}
}

void ProductionManager::addNewUnit(const Unit *unit)
{
	if (blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].ability_id == productionQueue.getNextItem())
	{
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