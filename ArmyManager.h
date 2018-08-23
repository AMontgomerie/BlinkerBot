#pragma once
#include <iostream>
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"
#include "UnitData.h"
#include <sstream>

using namespace sc2;

class BlinkerBot;

class ArmyManager
{
	BlinkerBot & blinkerBot;

	enum ArmyStatus { Defend, Attack, Retreat, Regroup };

	const int LOCALRADIUS = 15;

	struct ArmyUnit
	{
		const Unit *unit;
		ArmyStatus status;
		ArmyUnit(const Unit *armyUnit, ArmyStatus unitStatus): unit(armyUnit), status(unitStatus){};
	};

	ArmyStatus currentStatus;
	std::vector<ArmyUnit> army;
	std::set<const Unit *> pulledWorkers;
	std::set<const Unit *> enemyArmy;
	std::set<const Unit *> enemyStructures;
	bool regroupComplete;
	bool enemyBaseExplored;
	bool warpgate;
	Point2D rallyPoint;

	bool regroup();
	void attack();
	void attack(Point2D target);
	void retreat();
	bool canAttack();
	float calculateSupply(std::set<const Unit *> army);
	float calculateSupply(std::vector<ArmyUnit> army);
	float calculateSupplyInRadius(Point2D centre, std::set<const Unit *> army);
	float calculateSupplyInRadius(Point2D centre, std::vector<ArmyUnit> army);
	float calculateSupplyAndWorkersInRadius(Point2D centre, std::set<const Unit *> army);
	const Unit *getClosestEnemy(const Unit *ourUnit);
	const Unit *getClosestEnemy(Point2D point);
	const Unit *getClosestEnemyBase(const Unit *ourUnit);
	bool inRange(const Unit *attacker, const Unit *target);
	bool inRange(const Unit *attacker, Point2D target);
	bool shieldsCritical(const Unit *unit, const Unit *attacker);
	bool blink(const Unit *unit);
	void printDebug();
	bool kite(ArmyUnit armyUnit);
	bool outranges(const Unit *attacker, const Unit *target);
	Point2D getRetreatPoint(const Unit *unit);
	void workerDefence();
	int calculateEnemyStaticDefence();
	float averageUnitDistanceToEnemyBase();
	const Unit *getClosestBase(const Unit *unit);
	const Unit *getClosestBase(Point2D point);
public:
	ArmyManager(BlinkerBot & bot);
	~ArmyManager();
	void onStep();
	void addUnit(const Unit *unit);
	void removeUnit(const Unit *unit);
	void addEnemyUnit(const Unit *unit);
	void removeEnemyUnit(const Unit *unit);
	void addEnemyStructure(const Unit *structure);
	void removeEnemyStructure(const Unit *structure);
	bool sendAttackSignal();
	void setRallyPoint(Point2D point);
	bool detectionRequired();
	void initialise();
	const Unit *underAttack();
	void warpgateComplete();
};

