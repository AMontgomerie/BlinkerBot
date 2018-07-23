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
	if (blinkerBot.Observation()->GetGameLoop() % 30 == 0)
	{
		if (canAttack() && regroupComplete)
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
	}
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

void ArmyManager::attack()
{
	for (auto unit : army)
	{
		blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, blinkerBot.Observation()->GetGameInfo().enemy_start_locations.front());
	}
}

void ArmyManager::retreat()
{
	for (auto unit : army)
	{
		blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::MOVE , blinkerBot.Observation()->GetStartLocation());
	}
}

void ArmyManager::addUnit(const Unit *unit)
{
	army.insert(unit);
}

void ArmyManager::removeUnit(const Unit *unit)
{
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
	if (calculateSupply(army) > calculateSupply(enemyArmy))
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