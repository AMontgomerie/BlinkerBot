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
	std::set<const Unit *> enemyArmy;
	bool regroupComplete;
	Point2D rallyPoint;

	const Unit *underAttack();
	bool regroup();
	void attack();
	void attack(Point2D target);
	void retreat();
	bool canAttack();
	float calculateSupply(std::set<const Unit *> army);
	void micro();
	const Unit *getClosestEnemy(const Unit *ourUnit);
	bool inRange(const Unit *attacker, const Unit *target);
	bool shieldsCritical(const Unit *unit, const Unit *attacker);
	void blink(const Unit *unit);
	void printDebug();
public:
	ArmyManager(BlinkerBot & bot);
	~ArmyManager();
	void onStep();
	void addUnit(const Unit *unit);
	void removeUnit(const Unit *unit);
	void addEnemyUnit(const Unit *unit);
	void removeEnemyUnit(const Unit *unit);
	bool sendAttackSignal();
	void receiveRallyPoint(Point2D point);
};

