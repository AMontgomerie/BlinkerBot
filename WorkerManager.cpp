#include "WorkerManager.h"
#include "Blinkerbot.h"


WorkerManager::WorkerManager(BlinkerBot & bot) : blinkerBot(bot)
{
}


WorkerManager::~WorkerManager()
{
}


void WorkerManager::addWorker(const Unit *unit)
{
	workers.insert(unit);
}

void WorkerManager::removeWorker(const Unit *unit)
{
	workers.erase(unit);
}

size_t WorkerManager::getWorkerCount()
{
	return workers.size();
}

void WorkerManager::checkGas(const Unit *gas)
{
	//check how many workers are mining at the geyser
	int toAdd = gas->ideal_harvesters - gas->assigned_harvesters;
	std::set<const Unit *>::const_iterator worker = workers.begin();

	//if we don't have enough, then put some more in
	while (toAdd > 0 && worker != workers.end())
	{
		//get a worker that isn't already mining gas or building a structure
		if (isAvailableWorker(*worker) &&
			((*worker)->orders.empty() || !UnitData::isBuildAbility((*(*worker)->orders.begin()).ability_id)))
		{
			blinkerBot.Actions()->UnitCommand(*worker, ABILITY_ID::SMART, gas);
			toAdd--;
		}
		worker++;
	}
	worker = workers.begin();
	//if we have too many then take some out
	while (toAdd < 0 && worker != workers.end())
	{
		for (auto order : (*worker)->orders)
		{
			if (order.target_pos == gas->pos || Distance2D((*worker)->pos, gas->pos) < 2)
			{
				returnToMining(*worker);
				toAdd++;
			}
		}
		worker++;
	}
}

//returns true if the worker is not mining gas
bool WorkerManager::isAvailableWorker(const Unit *unit)
{
	if (unit->orders.empty())
	{
		return true;
	}
	else
	{
		//if the worker's current target is a townhall, then it might be returning vespene gas
		for (auto base : bases)
		{
			if (base->tag == unit->orders.front().target_unit_tag)
			{
				return false;
			}
		}
		//if the worker's current target is a gas geyser, then it's not available
		for (auto gas : gases)
		{
			if (gas->tag == unit->orders.front().target_unit_tag)
			{
				return false;
			}
		}
		return true;
	}
}

const Unit *WorkerManager::getBuilder()
{
	if (workers.size() > 0)
	{
		std::set<const Unit *>::const_iterator builder = workers.begin();
		while (!isAvailableWorker(*builder))
		{
			builder++;
		}
		return *builder;
	}
	else
	{
		std::cerr << "No workers Available" << std::endl;
		return nullptr;
	}
}

/*
checks our mineral saturation at each nexus, and calls transferWorkers if we are over-saturated
*/
void WorkerManager::checkSaturation()
{
	for (auto base : bases)
	{
		for (auto structure : blinkerBot.Observation()->GetUnits())
		{
			//check for our structures
			if (UnitData::isStructure(structure) && UnitData::isOurs(structure))
			{
				if (Distance2D(base->pos, structure->pos) < 4 && structure->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
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
}

/*
takes an oversaturated base and the amount of workers we want to move and tries to find another place for them to go
*/
void WorkerManager::transferWorkers(int numOfWorkers, const Unit *overSaturatedBase)
{
	for (auto base : bases)
	{
		//make sure we're looking at a different base location
		if (base != overSaturatedBase)
		{
			for (auto structure : blinkerBot.Observation()->GetUnits())
			{
				//check for our structures
				if (UnitData::isStructure(structure) && UnitData::isOurs(structure))
				{
					//find the nexus for this base
					if (Distance2D(base->pos, structure->pos) < 4 && structure->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
					{
						//std::cerr << "we found another nexus" << std::endl;
						//calculate how many workers we can take on
						int freeSpace = structure->ideal_harvesters - structure->assigned_harvesters;
						std::set<const Unit *>::iterator worker = workers.begin();
						while (freeSpace > 0 && numOfWorkers > 0 && worker != workers.end())
						{
							//std::cerr << "found an undersaturated nexus" << std::endl;
							if (Distance2D((*worker)->pos, overSaturatedBase->pos) < 5 && isAvailableWorker(*worker))
							{
								//std::cerr << "transferring a worker" << std::endl;

								blinkerBot.Actions()->UnitCommand((*worker), ABILITY_ID::MOVE, base->pos);
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
}

/*
check if any of our bases are starting to run out of resources
*/
bool WorkerManager::miningOut()
{
	for (auto base : bases)
	{
		int mineralNodeCount = 0;
		for (auto unit : blinkerBot.Observation()->GetUnits())
		{
			//count how many mineral patches still have remaining minerals
			if (unit->mineral_contents > 0)
			{
				mineralNodeCount++;
			}
		}
		if (mineralNodeCount < 7)
		{
			//std::cerr << "we're mining out!" << std::endl;
			return true;
		}
	}
	return false;
}

void WorkerManager::update()
{
	checkForIdleWorkers();
	checkSaturation();
	checkGases();

	/*
	std::ostringstream workerCount;
	workerCount << "gas worker count: " << gasWorkers.size() << std::endl;
	blinkerBot.Debug()->DebugTextOut(workerCount.str());
	blinkerBot.Debug()->SendDebug();
	*/
}

/*
calls returnToMining for any idle workers
*/
void WorkerManager::checkForIdleWorkers()
{
	for (auto worker : workers)
	{
		if (worker->orders.empty())
		{
			returnToMining(worker);
		}
	}
}

/*
checks the number of workers harvesting from each gas
*/
void WorkerManager::checkGases()
{
	for (auto gas : gases)
	{
		checkGas(gas);
	}
}

/*
tells a worker to mine minerals (called by BlinkerBot::OnUnitIdle)
*/
void WorkerManager::returnToMining(const Unit *unit)
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
	//if there's no visible minerals nearby, issue a move command towards the closest base
	//we issue move rather than smart here because smart can't right click on units with a display_type of snapshot (which might be the case here)
	if (!nextToMineral)
	{
		//if we don't have any bases then just move to a mineral node's location
		if (bases.empty())
		{
			for (auto unit : blinkerBot.Observation()->GetUnits())
			{
				if (UnitData::isMinerals(unit))
				{
					blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::MOVE, unit->pos);
				}
			}
		}
		//otherwise let's find our closest base and go there
		else
		{
			const Unit *closestBase = *bases.begin();
			for (auto base : bases)
			{
				if (Distance2D(unit->pos, base->pos) < Distance2D(unit->pos, closestBase->pos))
				{
					closestBase = base;
				}
			}
			//move towards the base
			blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::MOVE, closestBase->pos);
		}
	}
}

void WorkerManager::addBase(const Unit *unit)
{
	if (UnitData::isOurs(unit) && UnitData::isTownHall(unit))
	{
		bases.insert(unit);
	}
}

void WorkerManager::removeBase(const Unit *unit)
{
	bases.erase(unit);
}

void WorkerManager::addGas(const Unit *unit)
{
	gases.insert(unit);
}

void WorkerManager::removeGas(const Unit *unit)
{
	gases.erase(unit);
}