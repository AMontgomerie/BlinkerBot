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
enum ArmyStatus;

class ArmyManager
{
	BlinkerBot & blinkerBot;

	const int LOCALRADIUS = 15;

	struct ArmyUnit
	{
		const Unit *unit;
		ArmyStatus status;
		ArmyUnit(const Unit *armyUnit, ArmyStatus unitStatus): unit(armyUnit), status(unitStatus){};
	};

	bool regroupComplete;
	bool enemyBaseExplored;
	bool warpgate;
	int currentArmyValue;
	int currentEnemyArmyValue;
	ArmyStatus currentStatus;
	Point2D rallyPoint;
	std::vector<ArmyUnit> army;
	std::set<const Unit *> darkTemplars;
	std::set<const Unit *> enemyArmy;
	std::set<const Unit *> enemyStructures;
	std::set<const Unit *> observers;
	std::set<const Unit *> pulledWorkers;

public:
	ArmyManager(BlinkerBot & bot);
	void addEnemyStructure(const Unit *structure);
	void addEnemyUnit(const Unit *unit);
	void addUnit(const Unit *unit);
	bool behind();
	bool detectionRequired();
	ArmyStatus getArmyStatus();
	void onStep();
	void removeEnemyUnit(const Unit *unit);
	void removeEnemyStructure(const Unit *structure);
	void removeUnit(const Unit *unit);
	void setRallyPoint(Point2D point);
	const Unit *underAttack();
	void warpgateComplete();

private:
	void attack();
	void attack(Point2D target);
	float averageUnitDistanceToEnemyBase();
	bool blink(const Unit *unit);
	int calculateEnemyStaticDefence();
	float calculateEnemyStaticDefenceInRadius(Point2D centre);
	float calculateSupply(std::set<const Unit *> army);
	float calculateSupply(std::vector<ArmyUnit> army);
	float calculateSupplyAndWorkersInRadius(Point2D centre, std::set<const Unit *> army);
	float calculateSupplyInRadius(Point2D centre, std::set<const Unit *> army);
	float calculateSupplyInRadius(Point2D centre, std::vector<ArmyUnit> army);
	bool canAttack();
	void darkTemplarHarass();
	Point2D findAttackTarget(const Unit *unit);
	const Unit *getClosestBase(const Unit *unit);
	const Unit *getClosestBase(Point2D point);
	const Unit *getClosestEnemy(const Unit *ourUnit);
	const Unit *getClosestEnemy(Point2D point);
	const Unit *getClosestEnemyBase(const Unit *ourUnit);
	Point2D getRetreatPoint(const Unit *unit);
	bool inRange(const Unit *attacker, const Unit *target);
	bool inRange(const Unit *attacker, Point2D target);
	bool kite(ArmyUnit armyUnit);
	void moveObservers();
	bool outranges(const Unit *attacker, const Unit *target);
	void printDebug();
	bool regroup();
	void retreat();
	bool shieldsCritical(const Unit *unit, const Unit *attacker);
	void updateArmyValues();
	void workerDefence();

};

