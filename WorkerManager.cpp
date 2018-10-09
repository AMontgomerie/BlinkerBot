#include "WorkerManager.h"
#include "Blinkerbot.h"


WorkerManager::WorkerManager(BlinkerBot & bot) : 
	blinkerBot(bot), enemyRace(Race::Random), armyStatus(Retreat),
	scout(nullptr), enemyMain(nullptr), threatenedStructure(nullptr), 
	scouting(false), beingProxied(false), proxyScouted(false), 
	threat(0){}

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
	workers.erase(unit);
	pulledWorkers.erase(unit);
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
			//std::cerr << "WorkerManager::getBuilder(): No workers Available." << std::endl;
			return nullptr;
		}
		return *builder;
	}
	else
	{
		//std::cerr << "WorkerManager::getBuilder(): No workers Available." << std::endl;
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
	//handle scouting the enemy base
	if (scout->is_alive)
	{
		if (enemyMain)
		{
			harassWorkers();
		}
		else if (!enemyMain && !scouting)
		{
			scoutEnemyBases();
		}
	}

	//defending worker rushes and pulling workers
	if (armyStatus == Defend)
	{
		if (!threatenedStructure)
		{
			threatenedStructure = findThreatenedStructure();
			threat = 0;
		}
		else
		{
			threat = calculateThreatSupplyDifference();
		}

		checkForThreatenedWorkers();
	}
	else
	{
		threatenedStructure = nullptr;
		threat = 0;
	}

	updatePulledWorkers();

	if (blinkerBot.Observation()->GetGameLoop() % 30 == 0)
	{
		checkForIdleWorkers();
		checkSaturation();
		if (armyStatus != Defend)
		{
			checkGases();
		}
		
		//check for cannon rushes and proxy gates
		if (enemyRace != Race::Zerg && blinkerBot.Observation()->GetGameLoop() < 3000 && !beingProxied)
		{
			beingProxied = checkForProxy();
		}
		//if we're being proxied then let's try to find out where
		if (beingProxied && !proxyScouted)
		{
			if (!proxyScout)
			{
				assignProxyScout();
			}
			scoutForProxies();
		}
		//when we find the proxy
		if (proxyScouted && proxyScout && proxyScout->is_alive)
		{
			proxyHarass();
		}
		//in the event that we checked and couldn't find anything, go back to work
		if (proxyScout && proxyScouting && !proxyScouted && proxyScout->orders.empty())
		{
			returnToMining(proxyScout);
			beingProxied = false;
			workers.insert(proxyScout);
			proxyScout = nullptr;
		}
	}
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
			/*
			for (auto unit : blinkerBot.Observation()->GetUnits())
			{
				if (UnitData::isMinerals(unit))
				{
					blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::MOVE, unit->pos);
				}
			}
			*/
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
finds a worker to use for scouting proxies
*/
void WorkerManager::assignProxyScout()
{
	if (workers.empty())
	{
		return;
	}
	else
	{
		std::set<const Unit *>::iterator worker = workers.begin();
		proxyScout = *worker;
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

	//ignore targets that go too far from the enemy base
	if (enemyMain && target && Distance2D(enemyMain->pos, target->pos) > LOCALRADIUS)
	{
		target = nullptr;
	}

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
(for worker harass purposes)
*/
const Unit *WorkerManager::getClosestEnemyWorker(const Unit *ourUnit)
{
	const Unit *closestEnemy = nullptr;

	for (auto enemy : blinkerBot.Observation()->GetUnits())
	{
		//if the unit is an enemy worker, we can see it now, and it's not going too far from the enemy base (we don't wanna chase a scout)
		if (UnitData::isWorker(enemy) && !UnitData::isOurs(enemy) && enemy->last_seen_game_loop == blinkerBot.Observation()->GetGameLoop())
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

/*
receives the current status of our army, so we know when to check if defensive workers need to be pulled
*/
void WorkerManager::setArmyStatus(ArmyStatus status)
{
	armyStatus = status;
}

/*
returns a value representing the size of the difference between the enemy's nearby army and ours.
Returns 0 if our army is bigger.
*/
float WorkerManager::calculateThreatSupplyDifference()
{
	float threat = 0;
	float ourSupply = 0;
	float enemySupply = 0;

	//if there's any enemy units threatening our base
	if (threatenedStructure)
	{
		for (auto unit : blinkerBot.Observation()->GetUnits())
		{
			//add all of our fighting units
			if (UnitData::isOurs(unit) && !UnitData::isWorker(unit) && !UnitData::isStructure(unit))
			{
				ourSupply += blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].food_required;
			}
			//add any nearby enemies
			else if (!UnitData::isOurs(unit) && Distance2D(unit->pos, threatenedStructure->pos) <= LOCALRADIUS)
			{
				enemySupply += blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].food_required;
			}
		}
	}

	threat = enemySupply - (1.3 * ourSupply);
	if (threat < 0)
	{
		return 0;
	}
	else
	{
		return threat;
	}
}

/*
returns a structure which is close to some enemy units
*/
const Unit *WorkerManager::findThreatenedStructure()
{
	for (auto unit : blinkerBot.Observation()->GetUnits())
	{
		if (!UnitData::isOurs(unit) && !UnitData::isMinerals(unit) && !UnitData::isVespeneGeyser(unit))
		{
			for (auto base : bases)
			{
				if (Distance2D(base->pos, unit->pos) <= LOCALRADIUS &&
					unit->last_seen_game_loop == blinkerBot.Observation()->GetGameLoop())
				{
					return base;
				}
			}
		}
	}
	return nullptr;
}

/*
handles workers that were pulled for defensive reasons
*/
void WorkerManager::updatePulledWorkers()
{
	//if we don't need to defend then send everyone back to work
	if (armyStatus != Defend || threat <= 0 || !threatenedStructure)
	{
		if (!pulledWorkers.empty())
		{
			for (auto worker : pulledWorkers)
			{
				returnToMining(worker);
				workers.insert(worker);
			}
			pulledWorkers.clear();
		}
	}
	else
	{
		int intThreat = std::round(threat);

		//add extra workers if we need to
		if (pulledWorkers.size() <= intThreat)
		{
			std::set<const Unit *>::iterator worker = workers.begin();
			while (pulledWorkers.size() <= intThreat && worker != workers.end())
			{
				if (threatenedStructure && Distance2D((*worker)->pos, threatenedStructure->pos) <= LOCALRADIUS && (*worker)->shield > 5)
				{
					pulledWorkers.insert(*worker);
					workers.erase(worker++);
				}
				else
				{
					++worker;
				}
			}
		}

		for (std::set<const Unit *>::iterator worker = pulledWorkers.begin(); worker != pulledWorkers.end();)
		{
			//return hurt workers to mining and remove them to the group
			if ((*worker)->shield < 5)
			{
				const Unit *mineral = getClosestVisibleMineral(threatenedStructure);
				if (mineral)
				{
					blinkerBot.Actions()->UnitCommand(*worker, ABILITY_ID::SMART, mineral);
				}
				else
				{
					returnToMining(*worker);
				}
				workers.insert(*worker);
				pulledWorkers.erase(worker++);
			}
			//tell our workers to attack
			else
			{
				//if workers chase too far away then pull them back
				if (threatenedStructure && Distance2D((*worker)->pos, threatenedStructure->pos) > LOCALRADIUS)
				{
					if ((*worker)->orders.empty() || (*worker)->orders.front().ability_id == ABILITY_ID::ATTACK)
					{
						const Unit *mineral = getClosestVisibleMineral(threatenedStructure);
						if (mineral)
						{
							blinkerBot.Actions()->UnitCommand(*worker, ABILITY_ID::SMART, mineral);
						}
						else
						{
							returnToMining(*worker);
						}
					}
				}
				//otherwise continue attacking
				else
				{
					const Unit *closestEnemy = getClosestEnemy(*worker);
					if (closestEnemy && ((*worker)->orders.empty() || (*worker)->orders.front().ability_id != ABILITY_ID::ATTACK))
					{
						blinkerBot.Actions()->UnitCommand(*worker, ABILITY_ID::ATTACK, closestEnemy->pos);
					}

					//if we can't find any enemies then go back to mining
					else if(!closestEnemy && (*worker)->orders.empty())
					{
						returnToMining(*worker);
					}
				}
				worker++;
			}
		}
	}
}

/*
finds the closest enemy to a given unit
*/
const Unit *WorkerManager::getClosestEnemy(const Unit *ourUnit)
{
	const Unit *closestEnemy = nullptr;
	for (auto unit : blinkerBot.Observation()->GetUnits())
	{
		if (!UnitData::isOurs(unit) && UnitData::canTarget(ourUnit, unit) && UnitData::isVisible(unit) 
			&& !UnitData::isMinerals(unit) && !UnitData::isVespeneGeyser(unit) && !UnitData::isNeutralRock(unit) 
			&& unit->unit_type != UNIT_TYPEID::ZERG_CREEPTUMORBURROWED)
		{
			if (!closestEnemy)
			{
				closestEnemy = unit;
			}
			else if (Distance2D(unit->pos, ourUnit->pos) < Distance2D(closestEnemy->pos, ourUnit->pos))
			{
				closestEnemy = unit;
			}
		}
	}
	return closestEnemy;
}

/*
returns the furthest visible mineral from a given worker (so it can mineral walk to escape)
*/
const Unit *WorkerManager::getFurthestVisibleMineral(const Unit *ourUnit)
{
	const Unit *furthestMineral = nullptr;
	for (auto unit : blinkerBot.Observation()->GetUnits())
	{
		if (UnitData::isMinerals(unit) && unit->display_type == Unit::DisplayType::Visible)
		{
			if (!furthestMineral)
			{
				furthestMineral = unit;
			}
			else if (Distance2D(ourUnit->pos, unit->pos) > Distance2D(ourUnit->pos, furthestMineral->pos))
			{
				furthestMineral = unit;
			}
		}
	}
	return furthestMineral;
}

/*
returns the closest visible mineral to a given unit
*/
const Unit *WorkerManager::getClosestVisibleMineral(const Unit *ourUnit)
{
	const Unit *closestMineral = nullptr;
	for (auto unit : blinkerBot.Observation()->GetUnits())
	{
		if (UnitData::isMinerals(unit) && unit->display_type == Unit::DisplayType::Visible)
		{
			if (!closestMineral)
			{
				closestMineral = unit;
			}
			else if (Distance2D(ourUnit->pos, unit->pos) < Distance2D(ourUnit->pos, closestMineral->pos))
			{
				closestMineral = unit;
			}
		}
	}
	return closestMineral;
}

/*
if any of our workers are under threat, make them run  or fight
*/
void WorkerManager::checkForThreatenedWorkers()
{
	for (auto worker : workers)
	{
		const Unit *enemy = getClosestEnemy(worker);

		if (enemy && threatenedStructure && 
			Distance2D(enemy->pos, worker->pos) < 2 && Distance2D(worker->pos, threatenedStructure->pos) <= 5)
		{
			if (worker->orders.empty() || worker->orders.front().ability_id != ABILITY_ID::ATTACK)
			{
				blinkerBot.Actions()->UnitCommand(worker, ABILITY_ID::ATTACK, enemy->pos);
			}
		}
		else if (worker->orders.empty() || worker->orders.front().ability_id == ABILITY_ID::ATTACK)
		{
			const Unit *mineral = nullptr;
			if (threatenedStructure)
			{
				mineral = getClosestVisibleMineral(threatenedStructure);
			}
			if (mineral)
			{
				blinkerBot.Actions()->UnitCommand(worker, ABILITY_ID::SMART, mineral);
			}
			else
			{
				returnToMining(worker);
			}
		}
	}
}

/*
returns the closest base to a given unit
*/
const Unit *WorkerManager::getClosestBase(const Unit *unit)
{
	const Unit *closestBase = nullptr;
	for (auto base : bases)
	{
		if (!closestBase)
		{
			closestBase = base;
		}
		else
		{
			if (Distance2D(unit->pos, base->pos) < Distance2D(unit->pos, closestBase->pos))
			{
				closestBase = base;
			}
		}
	}
	return closestBase;
}

/*
searches through all of our workers current orders and issues STOP commands to any workers that are performing the given action
*/
void WorkerManager::checkForDuplicateBuildCommands(AbilityID ability)
{
	for (auto worker : workers)
	{
		for (auto order : worker->orders)
		{
			if (order.ability_id == ability)
			{
				blinkerBot.Actions()->UnitCommand(worker, ABILITY_ID::STOP);
			}
		}
	}
}

/*
sends a worker to scout in the event that we suspect a proxy
*/
void WorkerManager::scoutForProxies()
{
	if (!proxyScout)
	{
		return;
	}

	//if our scout isn't scouting yet, let's get started
	if (!proxyScouting && (scout->orders.empty() || scout->orders.front().ability_id != ABILITY_ID::MOVE))
	{
		//queue up a move command to all the nearby bases
		for (auto base : baseLocations)
		{
			if (Distance2D(proxyScout->pos, base) < PROXYDIST && Distance2D(base, blinkerBot.Observation()->GetStartLocation()) > 10)
			{
				blinkerBot.Actions()->UnitCommand(proxyScout, ABILITY_ID::MOVE, base, true);
			}
			proxyScouting = true;
		}
		//return home after scouting the bases
		if (!bases.empty())
		{
			const Unit *home = *bases.begin();
			blinkerBot.Actions()->UnitCommand(proxyScout, ABILITY_ID::MOVE, home->pos, true);
		}
	}

	//check the enemy bases we know about, if they're close to our base, we've found the proxy
	for (auto structure : enemyStructures)
	{
		if (Distance2D(blinkerBot.Observation()->GetStartLocation(), structure->pos) < PROXYDIST)
		{
			proxiedEnemyStructures.insert(structure);
			proxyScouted = true;
		}
	}
}

/*
receives a set of base locations for scouting purposes
*/
void WorkerManager::setBaseLocations(std::vector<Point2D> bases)
{
	baseLocations = bases;
}

/*
adds new enemy bases
*/
void WorkerManager::addEnemyStructure(const Unit *unit)
{
	enemyStructures.insert(unit);
}

/*
sets our opponent's race when we discover it
*/
void WorkerManager::setEnemyRace(Race race)
{
	enemyRace = race;
}

/*
returns true vs opponents that open with a forge
*/
bool WorkerManager::checkForProxy()
{
	if (enemyStructures.empty())
	{
		return false;
	}

	//count enemy structures
	int enemyBases = 0;
	int productionFacilities = 0;
	bool forge = false;
	for (auto structure : enemyStructures)
	{
		if (structure->unit_type == UNIT_TYPEID::PROTOSS_FORGE)
		{
			forge = true;
		}
		else if (UnitData::isTownHall(structure))
		{
			enemyBases++;
		}
		else if (structure->unit_type == UNIT_TYPEID::PROTOSS_GATEWAY)
		{
			productionFacilities++;
		}
	}

	//we count it as a proxy when we can see a nexus and either nothing or just a forge
	if ((enemyBases < 2 && forge) ||
		(blinkerBot.Observation()->GetGameLoop() > 1500 && enemyBases < 2 && productionFacilities < 1))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
if an enemy structure is destroyed, remove it from the set if its a proxy we're keeping track of
*/
void WorkerManager::removeEnemyProxy(const Unit *unit)
{
	proxiedEnemyStructures.erase(unit);
}

/*
if we find an enemy proxy with our scout, try to harass the worker and the structures
*/
void WorkerManager::proxyHarass()
{
	//try to find the enemy worker building the proxy
	const Unit *enemy = getClosestEnemyWorker(proxyScout);

	//ignore any workers that are too far from the proxy
	if (!proxiedEnemyStructures.empty() && enemy)
	{
		const Unit *proxy = *proxiedEnemyStructures.begin();
		if (Distance2D(proxy->pos, enemy->pos) > LOCALRADIUS)
		{
			enemy = nullptr;
		}
	}
	//if we've found an enemy, attack them
	if (enemy && (proxyScout->orders.empty() || proxyScout->orders.front().target_unit_tag != enemy->tag))
	{
		blinkerBot.Actions()->UnitCommand(proxyScout, ABILITY_ID::ATTACK, enemy);
	}
	//if we can't find any enemies, attack towards the enemy proxy
	else if (!proxiedEnemyStructures.empty() &&
		(proxyScout->orders.empty() || proxyScout->orders.front().ability_id != ABILITY_ID::ATTACK))
	{
		blinkerBot.Actions()->UnitCommand(proxyScout, ABILITY_ID::ATTACK, (*proxiedEnemyStructures.begin())->pos);
	}
}