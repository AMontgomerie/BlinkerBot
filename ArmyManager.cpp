#include "ArmyManager.h"
#include "Blinkerbot.h"

ArmyManager::ArmyManager(BlinkerBot & bot) : blinkerBot(bot), currentStatus(Defend), regroupComplete(true), enemyBaseExplored(false)
{
}


ArmyManager::~ArmyManager()
{
}


void ArmyManager::initialise()
{
	scout();
}

/*
//finds a worker and sends it to each potential enemy start location
*/
void ArmyManager::scout()
{
	const Unit *worker = *blinkerBot.Observation()->GetUnits().begin();
	const Unit *mineral = nullptr;

	for (auto unit : blinkerBot.Observation()->GetUnits())
	{
		//find one of our workers
		if (UnitData::isWorker(unit))
		{
			worker = unit;
		}
		//find a mineral node in our main
		if (UnitData::isMinerals(unit) && unit->display_type == Unit::DisplayType::Visible 
			&& Distance2D(unit->pos, blinkerBot.Observation()->GetStartLocation()) < 15)
		{
			mineral = unit;
		}
	}
	//queue commands to move to each potential enemy start location
	for (auto location : blinkerBot.Observation()->GetGameInfo().enemy_start_locations)
	{
		blinkerBot.Actions()->UnitCommand(worker, ABILITY_ID::MOVE, location, true);
	}
	//queue a command to return to mining after scouting
	blinkerBot.Actions()->UnitCommand(worker, ABILITY_ID::SMART, mineral->pos, true);
}


void ArmyManager::onStep()
{
	/*
	//timer stuff
	std::clock_t start;
	double duration;
	start = std::clock();
	//timer stuff
	*/

	workerDefence();
	if (blinkerBot.Observation()->GetGameLoop() % 30 == 0 && underAttack())
	{
		//std::cerr << "under attack" << std::endl;
		attack(underAttack()->pos);
	}
	else if (blinkerBot.Observation()->GetGameLoop() % 30 == 0 && canAttack() && regroupComplete)
	{
		//std::cerr << "attacking" << std::endl;
		currentStatus = Attack;
	}
	else
	{
		if (blinkerBot.Observation()->GetGameLoop() % 30 == 0)
		{
			//std::cerr << "regrouping" << std::endl;
			regroupComplete = regroup();
			currentStatus = Regroup;
		}
	}

	switch (currentStatus)
	{
	case Attack:
		if (blinkerBot.Observation()->GetGameLoop() % 120 == 0)
		{
			//std::cerr << "attack!" << std::endl;
		}
		attack();
		break;
	case Retreat:
		if (blinkerBot.Observation()->GetGameLoop() % 120 == 0)
		{
			//std::cerr << "retreat!" << std::endl;
		}
		retreat();
		break;
	case Defend:
		if (blinkerBot.Observation()->GetGameLoop() % 120 == 0)
		{
			//std::cerr << "defend!" << std::endl;
		}
		//not yet implemented
		break;
	}

	/*
	//timer stuff
	duration = (std::clock() - start) / (double)CLOCKS_PER_SEC * 1000;
	if (duration > 20)
	{
		std::cout << "ArmyManager duration: " << duration << '\n';
	}
	//timer stuff
	*/
}

/*
checks if any enemy units are near our bases and returns a unit pointer to the base that is under threat
*/
const Unit *ArmyManager::underAttack()
{
	for (auto enemy : enemyArmy)
	{
		for (auto unit : blinkerBot.Observation()->GetUnits())
		{
			//if there is an enemy in range of one of our structures
			if (UnitData::isOurs(unit) && UnitData::isStructure(unit) && 
				(enemy->last_seen_game_loop == blinkerBot.Observation()->GetGameLoop() && 
				(inRange(enemy, unit) || Distance2D(enemy->pos, unit->pos) < 15)))
			{
				//std::cerr << UnitTypeToName(unit->unit_type) << " is under attack from " << UnitTypeToName(enemy->unit_type) << std::endl;
				return unit;
			}
		}
	}
	return nullptr;
}

/*
pulls workers to defend in the event that there are not enough fighting units
*/
void ArmyManager::workerDefence()
{
	//if we don't have enough fighting units then we need to pull some workers
	if (underAttack() && (calculateSupply(enemyArmy) > calculateSupply(army) * 1.2))
	{
		const Unit *threatenedStructure = underAttack();
		//if we don't have any workers then let's find some
		if (pulledWorkers.empty())
		{
			/*
			//calculate the size of the enemy army
			int threatLevel = 2 * int(calculateSupply(enemyArmy));
			*/
			//calculate the threat level of the nearby enemy units
			std::set<const Unit *> enemies;
			for (auto enemy : enemyArmy)
			{
				if (Distance2D(enemy->pos, threatenedStructure->pos) < 20)
				{
					enemies.insert(enemy);
				}
			}
			int threatLevel = 2 * int(calculateSupply(enemies));

			//find nearby workers
			for (auto unit : blinkerBot.Observation()->GetUnits())
			{
				if (UnitData::isOurs(unit) && UnitData::isWorker(unit) && 
					Distance2D(unit->pos, threatenedStructure->pos) < 20 && threatLevel > 0)
				{
					pulledWorkers.insert(unit);
					threatLevel--;
				}
			}
		}
		//if we've already got some workers, then let's make sure they're being pulled
		for (auto worker : pulledWorkers)
		{
			if (worker->orders.empty() || (*worker->orders.begin()).ability_id != ABILITY_ID::ATTACK)
			{
				blinkerBot.Actions()->UnitCommand(worker, ABILITY_ID::ATTACK, getClosestEnemy(worker));
			}
		}
	}
	//if we're not under attack then we don't need the workers anymore
	else if (!pulledWorkers.empty() && 
		(getClosestEnemy(*pulledWorkers.begin()) == nullptr ||
		Distance2D(getClosestEnemy(*pulledWorkers.begin())->pos, (*pulledWorkers.begin())->pos) > 20))
	{
		pulledWorkers.clear();
	}

	//if workers chase too far away from our bases then tell them to go back home
	for (auto worker : pulledWorkers)
	{
		const Unit *closestBase = getClosestBase(worker);
		if (closestBase && Distance2D(closestBase->pos, worker->pos) > 20)
		{
			blinkerBot.Actions()->UnitCommand(worker, ABILITY_ID::MOVE, closestBase->pos);
		}
	}
}


bool ArmyManager::regroup()
{
	if (army.size() > 0)
	{
		//tell units to go to the regroup point
		Point2D regroupPoint = rallyPoint;
		bool unitsCloseEnough = true;
		for (auto armyUnit : army)
		{
			if (Distance2D(armyUnit.unit->pos, regroupPoint) > 10.0f)
			{
				unitsCloseEnough = false;
				blinkerBot.Actions()->UnitCommand(armyUnit.unit, ABILITY_ID::MOVE, regroupPoint);
			}
		}
		return unitsCloseEnough;
	}
	else
	{
		//std::cerr << "no units to regroup" << std::endl;
		return false;
	}
}

//tell all our units to attack move towards the enemy
void ArmyManager::attack()
{
	for (auto armyUnit : army)
	{
		Point2D target;

		//confirms that we have visited the enemy base
		if (Distance2D(armyUnit.unit->pos, blinkerBot.Observation()->GetGameInfo().enemy_start_locations.front()) < 8)
		{
			enemyBaseExplored = true;
		}

		if (enemyStructures.empty())
		{
			//if we've been to the enemy main but don't know the location of any enemy bases, set a random location
			if (enemyBaseExplored)
			{
				target = Point2D(float(GetRandomInteger(0, blinkerBot.Observation()->GetGameInfo().width)), float(GetRandomInteger(0, blinkerBot.Observation()->GetGameInfo().height)));
			}
			//if we haven't seen any enemy bases yet, let's just attack towards his start location
			else
			{
				target = blinkerBot.Observation()->GetGameInfo().enemy_start_locations.front();
			}
		}
		//otherwise let's find his closest base and go there
		else
		{
			target = getClosestEnemyBase(armyUnit.unit)->pos;
		}

		//if we don't know where the enemy base is, let's search randomly with our idle units
		if (enemyStructures.empty() && enemyBaseExplored)
		{
			if (armyUnit.unit->orders.empty())
			{
				blinkerBot.Actions()->UnitCommand(armyUnit.unit, ABILITY_ID::ATTACK, target);
			}
		}
		else
		{
			bool retreating = false;
			if (calculateSupply(army) < 50)
			{
				//if any nearby units are trying to kite, let's run back too so we don't get in their way
				for (auto otherUnit : army)
				{
					if (otherUnit.unit != armyUnit.unit && otherUnit.status == Retreat
						&& Distance2D(armyUnit.unit->pos, otherUnit.unit->pos) < 8 && armyUnit.unit->weapon_cooldown > 0)
					{
						retreating = true;
					}
				}
			}
			//we don't want to attack-move observers, so let's tell them to follow one of our units instead
			if (armyUnit.unit->unit_type == UNIT_TYPEID::PROTOSS_OBSERVER)
			{
				blinkerBot.Actions()->UnitCommand(armyUnit.unit, ABILITY_ID::SMART, (*army.begin()).unit);
			}
			else if (retreating || getClosestEnemyBase(armyUnit.unit) &&
				Distance2D(armyUnit.unit->pos, getClosestEnemyBase(armyUnit.unit)->pos) + 20 < averageUnitDistanceToEnemyBase())
			{
				//if (armyUnit.unit->orders.empty() || (*armyUnit.unit->orders.begin()).ability_id != ABILITY_ID::MOVE)
				//{
					blinkerBot.Actions()->UnitCommand(armyUnit.unit, ABILITY_ID::MOVE, getRetreatPoint(armyUnit.unit));
				//}
			}
			else
			{
				bool alreadyAttacking = false;
				if (armyUnit.unit->orders.size() > 0 && armyUnit.unit->orders.front().ability_id == ABILITY_ID::ATTACK)
				{
					alreadyAttacking = true;
				}
				if (armyUnit.unit->unit_type == UNIT_TYPEID::PROTOSS_STALKER)
				{
					if (!blink(armyUnit.unit) && !kite(armyUnit) && !alreadyAttacking)
					{
						armyUnit.status = Attack;
						if (armyUnit.unit->orders.empty() || (*armyUnit.unit->orders.begin()).ability_id != ABILITY_ID::ATTACK)
						{
							blinkerBot.Actions()->UnitCommand(armyUnit.unit, ABILITY_ID::ATTACK, target);
						}
					}
				}
				else
				{
					if (!kite(armyUnit) && !alreadyAttacking)
					{
						armyUnit.status = Attack;
						if (armyUnit.unit->orders.empty() || (*armyUnit.unit->orders.begin()).ability_id != ABILITY_ID::ATTACK)
						{
							blinkerBot.Actions()->UnitCommand(armyUnit.unit, ABILITY_ID::ATTACK, target);
						}
					}
				}
			}
		}
	}
}

/*
returns the average distance of our army units to the enemy base
*/
float ArmyManager::averageUnitDistanceToEnemyBase()
{
	float distance = 0;

	for (auto armyUnit : army)
	{
		distance += Distance2D(armyUnit.unit->pos, getClosestEnemyBase(armyUnit.unit)->pos);
	}

	distance = distance / army.size();

	return distance;
}

//tell all our units to attack move towards a specific point on the map
void ArmyManager::attack(Point2D target)
{
	for (auto armyUnit : army)
	{
		//we don't want to attack-move observers, so let's tell them to follow one of our units instead
		if (armyUnit.unit->unit_type == UNIT_TYPEID::PROTOSS_OBSERVER)
		{
			blinkerBot.Actions()->UnitCommand(armyUnit.unit, ABILITY_ID::SMART, (*army.begin()).unit);
		}
		else if (armyUnit.unit->unit_type == UNIT_TYPEID::PROTOSS_STALKER)
		{
		if (!blink(armyUnit.unit) && !kite(armyUnit))
		{
			armyUnit.status = Attack;
			if (armyUnit.unit->orders.empty() || (*armyUnit.unit->orders.begin()).ability_id != ABILITY_ID::ATTACK)
			{
				blinkerBot.Actions()->UnitCommand(armyUnit.unit, ABILITY_ID::ATTACK, target);
			}
		}
		}
		else
		{
			if (!kite(armyUnit))
			{
				armyUnit.status = Attack;
				if (armyUnit.unit->orders.empty() || (*armyUnit.unit->orders.begin()).ability_id != ABILITY_ID::ATTACK)
				{
					blinkerBot.Actions()->UnitCommand(armyUnit.unit, ABILITY_ID::ATTACK, target);
				}
			}
		}
	}
}

void ArmyManager::retreat()
{
	for (auto armyUnit : army)
	{
		blinkerBot.Actions()->UnitCommand(armyUnit.unit, ABILITY_ID::MOVE, rallyPoint);
	}
}

void ArmyManager::addUnit(const Unit *unit)
{
	for (auto armyUnit : army)
	{
		if (armyUnit.unit == unit)
		{
			return;
		}
	}
	army.push_back(ArmyUnit(unit, Attack));
}

void ArmyManager::removeUnit(const Unit *unit)
{
	for (std::vector<ArmyUnit>::iterator armyUnit = army.begin(); armyUnit != army.end();)
	{
		if ((*armyUnit).unit == unit)
		{
			armyUnit = army.erase(armyUnit);
		}
		else
		{
			++armyUnit;
		}
	}
	for (std::set<const Unit*>::iterator worker = pulledWorkers.begin(); worker != pulledWorkers.end();)
	{
		if (*worker == unit)
		{
			pulledWorkers.erase(worker++);
		}
		else
		{
			++worker;
		}
	}
}

void ArmyManager::addEnemyUnit(const Unit *unit)
{
	bool alreadySeen = false;
	for (auto enemyUnit : enemyArmy)
	{
		if (enemyUnit == unit)
		{
			alreadySeen = true;
		}
	}
	if (!alreadySeen && !UnitData::isStructure(unit)) //&& !UnitData::isWorker(unit)
	{
		//std::cerr << "adding new " << blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].name << std::endl;
		enemyArmy.insert(unit);
	}
}

void ArmyManager::removeEnemyUnit(const Unit *unit)
{
	for (std::set<const Unit *>::iterator enemyUnit = enemyArmy.begin(); enemyUnit != enemyArmy.end();)
	{
		if ((*enemyUnit) == unit)
		{
			enemyArmy.erase(*enemyUnit++);
		}
		else
		{
			++enemyUnit;
		}
	}
}

/*
generates an integer representing the size of the threat that enemy static defence represents
*/
int ArmyManager::calculateEnemyStaticDefence()
{
	int total = 0;
	for (auto structure : enemyStructures)
	{
		if (structure->unit_type == UNIT_TYPEID::TERRAN_BUNKER ||
			structure->unit_type == UNIT_TYPEID::PROTOSS_PHOTONCANNON ||
			structure->unit_type == UNIT_TYPEID::ZERG_SPINECRAWLER)
		{
			total += 8;
		}
		else if (structure->unit_type == UNIT_TYPEID::TERRAN_PLANETARYFORTRESS)
		{
			total += 20;
		}
	}
	return total;
}

bool ArmyManager::canAttack()
{
	if (calculateSupply(army) > 1 
		&& (calculateSupply(army) >= calculateSupply(enemyArmy) + calculateEnemyStaticDefence() || 
			blinkerBot.Observation()->GetFoodCap() <= blinkerBot.Observation()->GetFoodUsed() + 30))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArmyManager::sendAttackSignal()
{
	if (currentStatus == Attack)
	{
		return true;
	}
	else
	{
		return false;
	}
}

float ArmyManager::calculateSupply(std::set<const Unit *> army)
{
	float total = 0;
	for (auto unit : army)
	{
		if (!UnitData::isWorker(unit))
		{
			total += blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].food_required;
		}
	}
	return total;
}

float ArmyManager::calculateSupply(std::vector<ArmyUnit> army)
{
	float total = 0;
	for (auto armyUnit : army)
	{
		if (!UnitData::isWorker(armyUnit.unit))
		{
			total += blinkerBot.Observation()->GetUnitTypeData()[armyUnit.unit->unit_type].food_required;
		}
	}
	return total;
}

/*
A forward pylon is used as a rally point. The position is determined by ProductionManager and passed via BlinkerBot.
*/
void ArmyManager::setRallyPoint(Point2D point)
{
	rallyPoint = point;

	//in the event that we have a forward pylon that we are using as a point to retreat to or regroup at...
	if ((currentStatus == Retreat || currentStatus == Regroup) &&
		(getClosestBase(rallyPoint) && Distance2D(getClosestBase(rallyPoint)->pos, rallyPoint) > 20))
	{
		//...if the enemy army is approaching the rally point, we don't want to fight...
		if (getClosestEnemy(rallyPoint) && Distance2D(getClosestEnemy(rallyPoint)->pos, rallyPoint) < 10)
		{
			//...so let's retreat to our closest base
			rallyPoint = getClosestBase(rallyPoint)->pos;
		}
	}
}

bool ArmyManager::kite(ArmyUnit armyUnit)
{
	//this function just causes lag and makes units bump into eachother in large fights
	if (calculateSupply(army) > 40)
	{
		return false;
	}
	//check if we are in range and if we outrange the enemy
	bool canKite = false;
	for (auto enemy : enemyArmy)
	{
		if (inRange(armyUnit.unit, enemy) && outranges(armyUnit.unit, enemy))
		{
			canKite = true;
		}
	}
	//if the enemy is kitable and we are on cooldown, run away
	if (canKite && armyUnit.unit->weapon_cooldown > 0)
	{
		armyUnit.status = Retreat;
		blinkerBot.Actions()->UnitCommand(armyUnit.unit, ABILITY_ID::MOVE, getRetreatPoint(armyUnit.unit));
		return true;
	}
	return false;
}

bool ArmyManager::outranges(const Unit *attacker, const Unit *target)
{
	if (!attacker || !target || blinkerBot.Observation()->GetUnitTypeData()[attacker->unit_type].weapons.size() == 0 
		|| blinkerBot.Observation()->GetUnitTypeData()[target->unit_type].weapons.size() == 0)
	{
		return false;
	}
	else if ((*blinkerBot.Observation()->GetUnitTypeData()[attacker->unit_type].weapons.begin()).range >
		(*blinkerBot.Observation()->GetUnitTypeData()[target->unit_type].weapons.begin()).range)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ArmyManager::blink(const Unit *unit)
{
	//check if any enemies are in range of our unit and if they can take out our shields
	bool threat = false;
	for (auto enemy : enemyArmy)
	{
		if (inRange(enemy, unit) && shieldsCritical(unit, enemy))
		{
			threat = true;
		}
	}
	if (threat && unit->unit_type == UNIT_TYPEID::PROTOSS_STALKER)
	{
		blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_BLINK_STALKER, getRetreatPoint(unit));
		return true;
	}
	return false;
}

/*
find the closest enemy army unit to a given unit
*/
const Unit *ArmyManager::getClosestEnemy(const Unit *ourUnit)
{
	if (enemyArmy.size() > 0)
	{
		const Unit *closestEnemy = *enemyArmy.begin();
		for (auto enemy : enemyArmy)
		{
			if (Distance2D(enemy->pos, ourUnit->pos) < Distance2D(closestEnemy->pos, ourUnit->pos))
			{
				closestEnemy = enemy;
			}
		}
		return closestEnemy;
	}
	else
	{
		return nullptr;
	}
}

/*
find the closest enemy army unit to a given point
*/
const Unit *ArmyManager::getClosestEnemy(Point2D point)
{
	if (enemyArmy.size() > 0)
	{
		const Unit *closestEnemy = *enemyArmy.begin();
		for (auto enemy : enemyArmy)
		{
			if (Distance2D(enemy->pos, point) < Distance2D(closestEnemy->pos, point))
			{
				closestEnemy = enemy;
			}
		}
		return closestEnemy;
	}
	else
	{
		return nullptr;
	}
}

/*
find the closest enemy structure to a given unit
*/
const Unit *ArmyManager::getClosestEnemyBase(const Unit *ourUnit)
{
	if (enemyStructures.empty())
	{
		return nullptr;
	}
	else
	{
		const Unit *closestStructure = *enemyStructures.begin();
		for (auto structure : enemyStructures)
		{
			if (Distance2D(ourUnit->pos, structure->pos) < Distance2D(ourUnit->pos, closestStructure->pos)
				&& structure->unit_type != UNIT_TYPEID::ZERG_CREEPTUMORBURROWED)
			{
				closestStructure = structure;
			}
		}
		return closestStructure;
	}
}

//returns true when attacker is in range of target
bool ArmyManager::inRange(const Unit *attacker, const Unit *target)
{
	//make sure we have an attacker and a target
	if (!attacker || !target || blinkerBot.Observation()->GetUnitTypeData()[attacker->unit_type].weapons.size() == 0)
	{
		return false;
	}
	//check if the attacker is in range with their main weapon
	else if (blinkerBot.Observation()->GetUnitTypeData()[attacker->unit_type].weapons.front().range >= Distance2D(attacker->pos, target->pos))
	{
		if (blinkerBot.Observation()->GetGameLoop() % 30 == 0)
		{
		//	std::cerr << UnitTypeToName(attacker->unit_type) << " range " << blinkerBot.Observation()->GetUnitTypeData()[attacker->unit_type].weapons.front().range << std::endl;
		}
		return true;
	}
	else
	{
		if (blinkerBot.Observation()->GetGameLoop() % 30 == 0)
		{
		//	std::cerr << UnitTypeToName(attacker->unit_type) << " range " << blinkerBot.Observation()->GetUnitTypeData()[attacker->unit_type].weapons.front().range << std::endl;
		}
		return false;
	}
}

//returns true if the next shot from the enemy will kill our unit's shields
bool ArmyManager::shieldsCritical(const Unit *unit, const Unit *attacker)
{
	if (!unit || !attacker || blinkerBot.Observation()->GetUnitTypeData()[attacker->unit_type].weapons.size() == 0)
	{
		return false;
	}
	//calculate the next attack damage as number of hits x damage per hit (doesn't factor in attack bonuses or upgrades)
	float nextAttackDamage = blinkerBot.Observation()->GetUnitTypeData()[attacker->unit_type].weapons.front().attacks * 
		blinkerBot.Observation()->GetUnitTypeData()[attacker->unit_type].weapons.front().damage_;
	//if the next attack is more than our shields, return true
	if (nextAttackDamage >= unit->shield)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ArmyManager::printDebug()
{
	/*
	std::ostringstream ourArmy;
	ourArmy << "Current army supply: " << calculateSupply(army) << std::endl;
	blinkerBot.Debug()->DebugTextOut(ourArmy.str(), Point2D(0.1f, 0.5f));
	std::ostringstream theirArmy;
	theirArmy << "Known enemy army supply: " << calculateSupply(enemyArmy) << std::endl;
	blinkerBot.Debug()->DebugTextOut(theirArmy.str(), Point2D(0.1f, 0.6f));
	blinkerBot.Debug()->SendDebug();

	std::string armyStatus;
	switch (currentStatus)
	{
	case Attack:
		armyStatus = "ATTACK";
			break;
	case Defend:
		armyStatus = "DEFEND";
		break;
	case Regroup:
		armyStatus = "REGROUP";
		break;
	case Retreat:
		armyStatus = "RETREAT";
		break;
	default:
		armyStatus = "UNKNOWN";
		break;
	}
	std::ostringstream statusString;
	statusString << "Army Status: " << armyStatus << std::endl;
	blinkerBot.Debug()->DebugTextOut(statusString.str());
	*/
}

void ArmyManager::addEnemyStructure(const Unit *structure)
{
	if (UnitData::isStructure(structure))
	{
		enemyStructures.insert(structure);
	}
}

void ArmyManager::removeEnemyStructure(const Unit *structure)
{
	for (std::set<const Unit *>::iterator enemyStructure = enemyStructures.begin(); enemyStructure != enemyStructures.end();)
	{
		if ((*enemyStructure) == structure)
		{
			enemyStructures.erase(*enemyStructure++);
		}
		else
		{
			++enemyStructure;
		}
	}
}

bool ArmyManager::detectionRequired()
{
	bool canCloak = false;
	//check if any enemy units we know about can potentially cloak
	for (auto unit : enemyArmy)
	{
		if (UnitData::canCloak(unit))
		{
			canCloak = true;
		}
	}
	//also check the enemy structures we know about to see if anything cloaked might be in production
	for (auto structure : enemyStructures)
	{
		if ((structure->unit_type == UNIT_TYPEID::PROTOSS_DARKSHRINE) ||
			(structure->unit_type == UNIT_TYPEID::TERRAN_GHOSTACADEMY) ||
			(structure->unit_type == UNIT_TYPEID::TERRAN_STARPORTTECHLAB) ||
			(structure->unit_type == UNIT_TYPEID::ZERG_INFESTATIONPIT) ||
			(structure->unit_type == UNIT_TYPEID::ZERG_LURKERDENMP))
		{
			canCloak = true;
		}
	}

	//check if we already have detection
	bool detected = false;
	for (auto armyUnit : army)
	{
		if (armyUnit.unit->unit_type == UNIT_TYPEID::PROTOSS_OBSERVER)
		{
			detected = true;
		}
	}
	if (!detected)
	{
		return canCloak;
	}
	else
	{
		return false;
	}
}

Point2D ArmyManager::getRetreatPoint(const Unit *unit)
{
	const int RETREATDIST = 5;
	Point2D retreatPoint = unit->pos;
	const Unit *closestEnemy = getClosestEnemy(unit);

	//if enemies are closer to our rally point than us, then let's run away from the enemy rather than to the rally point
	if (closestEnemy && Distance2D(closestEnemy->pos, rallyPoint) < Distance2D(unit->pos, rallyPoint))
	{
		if (closestEnemy->pos.x > unit->pos.x)
		{
			retreatPoint = Point2D(unit->pos.x - RETREATDIST, unit->pos.y);
		}
		else if (closestEnemy->pos.x < unit->pos.x)
		{
			retreatPoint = Point2D(unit->pos.x + RETREATDIST, unit->pos.y);
		}
		if (closestEnemy->pos.y > unit->pos.y)
		{
			retreatPoint = Point2D(unit->pos.x, unit->pos.y - RETREATDIST);
		}
		else if (closestEnemy->pos.y < unit->pos.y)
		{
			retreatPoint = Point2D(unit->pos.x, unit->pos.y + RETREATDIST);
		}
	}
	//otherwise let's just retreat back to our rally point
	else
	{
		retreatPoint = rallyPoint;
	}

	return retreatPoint;
}

/*
finds the closest townhall to a given unit
*/
const Unit *ArmyManager::getClosestBase(const Unit *unit)
{
	const Unit *closestBase = nullptr;
	for (auto structure : blinkerBot.Observation()->GetUnits())
	{
		if (UnitData::isOurs(structure) && UnitData::isTownHall(structure))
		{
			if (!closestBase || Distance2D(structure->pos, unit->pos) < Distance2D(closestBase->pos, unit->pos))
			{
				closestBase = structure;
			}
		}
	}
	return closestBase;
}

/*
finds the closest townhall to a given point
*/
const Unit *ArmyManager::getClosestBase(Point2D point)
{
	const Unit *closestBase = nullptr;
	for (auto structure : blinkerBot.Observation()->GetUnits())
	{
		if (UnitData::isOurs(structure) && UnitData::isTownHall(structure))
		{
			if (!closestBase || Distance2D(structure->pos, point) < Distance2D(closestBase->pos, point))
			{
				closestBase = structure;
			}
		}
	}
	return closestBase;
}