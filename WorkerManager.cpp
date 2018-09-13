#include "WorkerManager.h"
#include "Blinkerbot.h"


WorkerManager::WorkerManager(BlinkerBot & bot) : blinkerBot(bot), scout(nullptr), enemyMain(nullptr), scouting(false){}

/*
perform one-time only actions at the start of the game
*/
void WorkerManager::initialise()
{
	assignScout();
}

/*
add a new worker to our set of workers
*/
void WorkerManager::addWorker(const Unit *unit)
{
	//this is to prevent the scout from being reassigned to the set of workers and pulled back home
	if (unit != scout)
	{
		workers.insert(unit);
	}
}

/*
remove a worker from our set of workers
*/
void WorkerManager::removeWorker(const Unit *unit)
{
	for (std::set<const Unit *>::iterator worker = workers.begin(); worker != workers.end();)
	{
		if (*worker == unit)
		{
			workers.erase(worker++);
		}
		else
		{
			++worker;
		}
	}
}

/*
returns the number of workers we have
*/
size_t WorkerManager::getWorkerCount()
{
	return workers.size();
}

/*
checks that a gasis being mined by the ideal number of harvesters
*/
void WorkerManager::checkGas(const Unit *gas)
{
	//check how many workers are mining at the geyser
	int toAdd = gas->ideal_harvesters - gas->assigned_harvesters;

	if (workers.size() > toAdd)
	{
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
}

/*
returns true if the worker is not mining gas or scouting
*/
bool WorkerManager::isAvailableWorker(const Unit *unit)
{
	if (unit->orders.empty())
	{
		return true;
	}
	else
	{
		if (unit == scout)
		{
			return false;
		}
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

/*
finds an available worker
*/
const Unit *WorkerManager::getBuilder()
{
	if (workers.size() > 0)
	{
		std::set<const Unit *>::const_iterator builder = workers.begin();
		while (builder != workers.end() && !isAvailableWorker(*builder))
		{
			++builder;
		}
		if (builder == workers.end() || !isAvailableWorker(*builder))
		{
			std::cerr << "WorkerManager::getBuilder(): No workers Available." << std::endl;
			return nullptr;
		}
		return *builder;
	}
	else
	{
		std::cerr << "WorkerManager::getBuilder(): No workers Available." << std::endl;
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
						//calculate how many workers we can take on
						int freeSpace = structure->ideal_harvesters - structure->assigned_harvesters;
						if (!workers.empty())
						{
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
		if (mineralNodeCount < 6)
		{
			//std::cerr << "we're mining out!" << std::endl;
			return true;
		}
	}
	return false;
}

/*
call regularly to trigger various worker management-related methods
*/
void WorkerManager::update()
{
	if (enemyMain && scout->is_alive)
	{
		harassWorkers();
	}
	else if (!enemyMain && !scouting && scout->is_alive)
	{
		scoutEnemyBases();
	}

	if (blinkerBot.Observation()->GetGameLoop() % 30 == 0)
	{
		checkForIdleWorkers();
		checkSaturation();
		checkGases();
	}

	/*
	std::ostringstream workerCount;
	workerCount << " worker count: " << workers.size() << std::endl;
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
		if (worker->orders.empty() && worker != scout)
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
	int availableWorkers = int(workers.size());
	for (auto gas : gases)
	{
		if (availableWorkers > 9)
		{
			checkGas(gas);
		}
		availableWorkers -= 3;
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

/*
adds a new base to the set of bases under our control
*/
void WorkerManager::addBase(const Unit *unit)
{
	if (UnitData::isOurs(unit) && UnitData::isTownHall(unit))
	{
		bases.insert(unit);
	}
}

/*
removes a base from the set of bases under our control
*/
void WorkerManager::removeBase(const Unit *unit)
{
	bases.erase(unit);
}

/*
adds a new gas to our set of gases (used by checkGas() to maintain saturation)
*/
void WorkerManager::addGas(const Unit *unit)
{
	gases.insert(unit);
}

/*
remove a gas from the set of our gases
*/
void WorkerManager::removeGas(const Unit *unit)
{
	gases.erase(unit);
}

/*
finds a worker to send scouting
*/
void WorkerManager::assignScout()
{
	if (workers.empty())
	{
		return;
	}
	else
	{
		std::set<const Unit *>::iterator worker = workers.begin();
		scout = *worker;
		removeWorker(*worker);
	}
}

/*
returns our assigned scouting worker
*/
const Unit *WorkerManager::getScout()
{
	return scout;
}

/*
scout the enemy bases if we don't know where they are
*/
void WorkerManager::scoutEnemyBases()
{
	//queue commands to move to each potential enemy start location
	for (auto location : blinkerBot.Observation()->GetGameInfo().enemy_start_locations)
	{
		blinkerBot.Actions()->UnitCommand(scout, ABILITY_ID::MOVE, location, true);
	}
	//queue a command to return to mining after scouting
	scouting = true;
}

/*
assign a pointer to the enemy main when we find it (for scouting and worker harassment purposes)
*/
void WorkerManager::addEnemyMain(const Unit *unit)
{
	for (auto location : blinkerBot.Observation()->GetGameInfo().enemy_start_locations)
	{
		if (Distance2D(location, unit->pos) < 2)
		{
			enemyMain = unit;
		}
	}
}

/*
commands the assigned scout to harras enemy workers
*/
void WorkerManager::harassWorkers()
{
	const Unit *target = getClosestEnemyWorker(scout);

	if (!enemyMain)
	{
		return;
	}
	else
	{
		//if the scout still has shields, keep fighting
		if (scout->shield > 5)
		{
			if (target)
			{
				if (scout->orders.empty() || (!scout->orders.empty() && scout->orders.front().ability_id != ABILITY_ID::ATTACK))
				{
					blinkerBot.Actions()->UnitCommand(scout, ABILITY_ID::ATTACK, target);
				}
			}
			else
			{
				if (scout->orders.empty() || (!scout->orders.empty() && scout->orders.front().ability_id != ABILITY_ID::MOVE))
				{
					blinkerBot.Actions()->UnitCommand(scout, ABILITY_ID::MOVE, enemyMain->pos);
				}
			}
		}
		//if the scout is getting hurt, then let's run away
		else
		{
			//find a mineral node in our main
			const Unit *mineral = *blinkerBot.Observation()->GetUnits().begin();
			for (auto unit : blinkerBot.Observation()->GetUnits())
			{
				if (UnitData::isMinerals(unit) && unit->display_type == Unit::DisplayType::Visible
					&& Distance2D(unit->pos, blinkerBot.Observation()->GetStartLocation()) < 15)
				{
					mineral = unit;
				}
			}
			//right click the mineral (so the scout can move through other units)
			if (mineral->display_type == Unit::DisplayType::Visible)
			{
				if (!scout->orders.empty() && scout->orders.front().ability_id != ABILITY_ID::HARVEST_GATHER)
				{
					blinkerBot.Actions()->UnitCommand(scout, ABILITY_ID::SMART, mineral);
				}
			}
			//if we can't do the mineral trick because no minerals are visible, then just right click back home
			else
			{
				if (!scout->orders.empty() && scout->orders.front().ability_id != ABILITY_ID::MOVE)
				{
					blinkerBot.Actions()->UnitCommand(scout, ABILITY_ID::MOVE, blinkerBot.Observation()->GetStartLocation());
				}
			}
		}
	}
}

/*
find the closest enemy worker to a given unit
*/
const Unit *WorkerManager::getClosestEnemyWorker(const Unit *ourUnit)
{
	const Unit *closestEnemy = nullptr;

	for (auto enemy : blinkerBot.Observation()->GetUnits())
	{
		//if the unit is an enemy worker, we can see it now, and it's not going too far from the enemy base (we don't wanna chase a scout)
		if (UnitData::isWorker(enemy) && !UnitData::isOurs(enemy) && enemy->last_seen_game_loop == blinkerBot.Observation()->GetGameLoop()
			&& enemyMain && Distance2D(enemyMain->pos, enemy->pos) < 30)
		{
			//make this enemy worker the closest if we don't have a closest or if it's closer than our previously recorded one
			if (!closestEnemy || (Distance2D(enemy->pos, ourUnit->pos) < Distance2D(closestEnemy->pos, ourUnit->pos)))
			{
				closestEnemy = enemy;
			}
		}
	}

	return closestEnemy;
}