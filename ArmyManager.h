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

	enum ArmyStatus {Defend, Attack, Retreat, Regroup};

	ArmyStatus currentStatus;
	std::set<const Unit *> army;
	std::set<const Unit *> kitingUnits;
	std::set<const Unit *> enemyArmy;
	std::set<const Unit *> enemyStructures;
	bool regroupComplete;
	bool enemyBaseExplored;
	Point2D rallyPoint;

	const Unit *underAttack();
	bool regroup();
	void attack();
	void attack(Point2D target);
	void retreat();
	bool canAttack();
	float calculateSupply(std::set<const Unit *> army);
	const Unit *getClosestEnemy(const Unit *ourUnit);
	bool inRange(const Unit *attacker, const Unit *target);
	bool shieldsCritical(const Unit *unit, const Unit *attacker);
	bool blink(const Unit *unit);
	void printDebug();
	bool kite(const Unit *unit);
	bool outranges(const Unit *attacker, const Unit *target);
	void addKitingUnit(const Unit *unit);
	void removeKitingUnit(const Unit *unit);
	Point2D getRetreatPoint(const Unit *unit);
	void workerDefence();
	void scout();
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
	void receiveRallyPoint(Point2D point);
	bool detectionRequired();
	void initialise();
};

