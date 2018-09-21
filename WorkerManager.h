#pragma once
#include <iostream>
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"
#include "UnitData.h"

using namespace sc2;

class BlinkerBot;
enum ArmyStatus;

class WorkerManager
{
	const int LOCALRADIUS = 30;
	BlinkerBot & blinkerBot;
	std::set<const Unit *> workers;
	std::set<const Unit *> pulledWorkers;
	std::set<const Unit *> bases;
	std::set<const Unit *> gases;
	const Unit *enemyMain;
	const Unit *scout;
	bool scouting;
	ArmyStatus armyStatus;
	const Unit *threatenedStructure;
	float threat;

	void returnToMining(const Unit *unit);
	void transferWorkers(int numOfWorkers, const Unit *overSaturatedBase);
	void checkSaturation();
	void checkForIdleWorkers();
	void checkGases();
	bool isAvailableWorker(const Unit *unit);
	void assignScout();
	void scoutEnemyBases();
	void harassWorkers();
	const Unit *getClosestEnemyWorker(const Unit *ourUnit);
	float calculateThreatSupplyDifference();
	const Unit *findThreatenedStructure();
	void updatePulledWorkers();
	const Unit *getClosestEnemy(const Unit *ourUnit);
	const Unit *getFurthestVisibleMineral(const Unit *ourUnit);
	const Unit *getClosestVisibleMineral(const Unit *ourUnit);
	void checkForThreatenedWorkers();
	const Unit *getClosestBase(const Unit *unit);
public:
	void initialise();
	void update();
	bool miningOut();
	size_t getWorkerCount();
	void checkGas(const Unit *gas);
	const Unit *getBuilder();
	void addWorker(const Unit *unit);
	void removeWorker(const Unit *unit);
	void addBase(const Unit *unit);
	void removeBase(const Unit *unit);
	void addGas(const Unit *unit);
	void removeGas(const Unit *unit);
	const Unit *getScout();
	void addEnemyMain(const Unit *unit);
	void setArmyStatus(ArmyStatus status);
	WorkerManager(BlinkerBot & bot);
};

