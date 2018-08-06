#include "ArmyManager.h"
#include "Blinkerbot.h"

ArmyManager::ArmyManager(BlinkerBot & bot) : blinkerBot(bot), currentStatus(Defend), regroupComplete(true)
{
}


ArmyManager::~ArmyManager()
{
}

void ArmyManager::onStep()
{
	//if (blinkerBot.Observation()->GetGameLoop() % 30 == 0)
	//{
		if (underAttack())
		{
			attack(underAttack()->pos);
		}
		else if (canAttack() && regroupComplete)
		{
			//std::cerr << "attacking" << std::endl;
			currentStatus = Attack;
		}
		else
		{
			//std::cerr << "regrouping" << std::endl;
			regroupComplete = regroup();
			currentStatus = Regroup;
		}

		switch (currentStatus)
		{
		case Attack:
			//std::cerr << "attack!" << std::endl;
			attack();
			break;
		case Retreat:
			//std::cerr << "retreat!" << std::endl;
			retreat();
			break;
		case Defend:
			//std::cerr << "defend!" << std::endl;
			//not yet implemented
			break;
		}
	//}
	//printDebug();
}

const Unit *ArmyManager::underAttack()
{
	for (auto enemy : enemyArmy)
	{
		for (auto unit : blinkerBot.Observation()->GetUnits())
		{
			//if there is an enemy in range of one of our structures
			if (UnitData::isOurs(unit) && UnitData::isStructure(unit) && inRange(enemy, unit))
			{
				return unit;
			}
		}
	}
	return nullptr;
}

bool ArmyManager::regroup()
{
	if (army.size() > 1)
	{/*
		Point2D regroupPoint = Point2D((*army.begin())->pos.x, (*army.begin())->pos.y);
		//get the unit that's closest to our base and use them as a regroup point
		for (auto unit : army)
		{
			if (Distance2D(unit->pos, blinkerBot.Observation()->GetStartLocation()) < Distance2D(regroupPoint, blinkerBot.Observation()->GetStartLocation()))
			{
				regroupPoint = unit->pos;
			}
		}
		*/
		//tell units to go to the regroup point
		Point2D regroupPoint = rallyPoint;
		bool unitsCloseEnough = true;
		for (auto unit : army)
		{
			if (Distance2D(unit->pos, regroupPoint) > 10.0f)
			{
				unitsCloseEnough = false;
				blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::MOVE, regroupPoint);
			}
		}
		return unitsCloseEnough;
	}
	else if (army.size() == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//tell all our units to attack move towards the enemy
void ArmyManager::attack()
{
	for (auto unit : army)
	{
		Point2D target;

		//if we haven't seen any enemy bases yet, let's just attack towards his start location
		if (enemyStructures.empty())
		{
			target = blinkerBot.Observation()->GetGameInfo().enemy_start_locations.front();
		}
		//otherwise let's find his closest base and go there
		else
		{
			const Unit *closestStructure = *enemyStructures.begin();
			for (auto structure : enemyStructures)
			{
				if (Distance2D(unit->pos, structure->pos) < Distance2D(unit->pos, closestStructure->pos))
				{
					closestStructure = structure;
				}
			}
			target = closestStructure->pos;
		}

		//if any nearby units are trying to kite, let's run back too so we don't get in their way
		for (auto kitingUnit : kitingUnits)
		{
			if (Distance2D(unit->pos, kitingUnit->pos) < 8 && unit->weapon_cooldown > 0)
			{
				blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::MOVE, getRetreatPoint(unit));
			}
		}

		if (unit->unit_type == UNIT_TYPEID::PROTOSS_STALKER)
		{
			if (!blink(unit) && !kite(unit))
			{
				removeKitingUnit(unit);
				blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, target);
			}
		}
		else
		{
			if (!kite(unit))
			{
				removeKitingUnit(unit);
				blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, target);
			}
		}
	}
}

//tell all our units to attack move towards a specific point on the map
void ArmyManager::attack(Point2D target)
{
	for (auto unit : army)
	{	
		if (unit->unit_type == UNIT_TYPEID::PROTOSS_STALKER)
		{
			if (!blink(unit) && !kite(unit))
			{
				blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, target);
			}
		}
		else
		{
			if (!kite(unit))
			{
				blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, target);
			}
		}
	}
}

void ArmyManager::retreat()
{
	for (auto unit : army)
	{
		blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::MOVE , rallyPoint);
	}
}

void ArmyManager::addUnit(const Unit *unit)
{
	army.insert(unit);
}

void ArmyManager::removeUnit(const Unit *unit)
{
	removeKitingUnit(unit);

	for (std::set<const Unit *>::iterator armyUnit = army.begin(); armyUnit != army.end();)
	{
		if ((*armyUnit) == unit)
		{
			army.erase(*armyUnit++);
		}
		else
		{
			++armyUnit;
		}
	}
}

void ArmyManager::addKitingUnit(const Unit *unit)
{
	kitingUnits.insert(unit);
}

void ArmyManager::removeKitingUnit(const Unit *unit)
{
	for (std::set<const Unit *>::iterator kitingUnit = kitingUnits.begin(); kitingUnit != kitingUnits.end();)
	{
		if ((*kitingUnit) == unit)
		{
			kitingUnits.erase(*kitingUnit++);
		}
		else
		{
			++kitingUnit;
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
	if (!alreadySeen && !UnitData::isWorker(unit) && !UnitData::isStructure(unit))
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

bool ArmyManager::canAttack()
{
	if (calculateSupply(army) > 1 
		&& (calculateSupply(army) >= calculateSupply(enemyArmy) || blinkerBot.Observation()->GetFoodCap() <= blinkerBot.Observation()->GetFoodUsed()))
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
		total += blinkerBot.Observation()->GetUnitTypeData()[unit->unit_type].food_required;
	}
	return total;
}

void ArmyManager::receiveRallyPoint(Point2D point)
{
	rallyPoint = point;
}

bool ArmyManager::kite(const Unit *unit)
{
	//check if we are in range and if we outrange the enemy
	bool canKite = false;
	for (auto enemy : enemyArmy)
	{
		if (inRange(unit, enemy) && outranges(unit, enemy))
		{
			canKite = true;
		}
	}
	//if the enemy is kitable and we are on cooldown, run away
	if (canKite && unit->weapon_cooldown > 0)
	{
		addKitingUnit(unit);
		blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::MOVE, getRetreatPoint(unit));
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
		return NULL;
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
	std::ostringstream ourArmy;
	ourArmy << "Current army supply: " << calculateSupply(army) << std::endl;
	blinkerBot.Debug()->DebugTextOut(ourArmy.str(), Point2D(0.1f, 0.5f));
	std::ostringstream theirArmy;
	theirArmy << "Known enemy army supply: " << calculateSupply(enemyArmy) << std::endl;
	blinkerBot.Debug()->DebugTextOut(theirArmy.str(), Point2D(0.1f, 0.6f));
	blinkerBot.Debug()->SendDebug();
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
	for (auto unit : army)
	{
		if (unit->unit_type == UNIT_TYPEID::PROTOSS_OBSERVER)
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
	const int RETREATDIST = 30;
	Point2D retreatPoint = unit->pos;
	const Unit *closestEnemy = getClosestEnemy(unit);

	//if enemies are closer to our rally point than us, then let's run away from the enemy rather than to the rally point
	if (Distance2D(closestEnemy->pos, rallyPoint) < Distance2D(unit->pos, rallyPoint))
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