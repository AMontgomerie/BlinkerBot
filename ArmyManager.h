#pragma once
#include <iostream>
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"
#include "UnitData.h"

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

	bool regroup();
	void attack();
	void retreat();
	bool canAttack();
	float calculateSupply(std::set<const Unit *> army);
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

