#pragma once
#include <iostream>
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"
#include "UnitData.h"

using namespace sc2;

class BlinkerBot;

class WorkerManager
{
	BlinkerBot & blinkerBot;
	std::set<const Unit *> workers;
	std::set<const Unit *> bases;
	std::set<const Unit *> gases;

	void returnToMining(const Unit *unit);
	void transferWorkers(int numOfWorkers, const Unit *overSaturatedBase);
	void checkSaturation();
	void checkForIdleWorkers();
	void checkGases();
	bool isAvailableWorker(const Unit *unit);
public:
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
	WorkerManager(BlinkerBot & bot);
	~WorkerManager();
};

