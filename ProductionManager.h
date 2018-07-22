#pragma once

#include <iostream>
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"
#include "ProductionQueue.h"

using namespace sc2;

class BlinkerBot;

class ProductionManager
{
	BlinkerBot & blinkerBot;
	ProductionQueue productionQueue;

	std::set<const Unit *> workers;
	std::set<const Unit *> availableWorkers;
	std::set<std::pair<const Unit *, std::set<const Unit *>>> gasWorkers;
	std::set<const Unit *> structures;
	std::set<const Unit *> availableGeysers;

	void buildStructure(AbilityID structureToBuild);
	const Unit *getClosestGas(const Unit *unit);
	const Unit *getClosestGas(Point3D point);
	void findGeysers();
	void produce(AbilityID nextBuildOrderItem);
	void trainUnits();
public:
	ProductionManager(BlinkerBot & bot);
	~ProductionManager();
	void initialise();
	void onStep();
	void onUnitDestroyed(const Unit *unit);
	void addWorker(const Unit *unit);
	void removeWorker(const Unit *unit);
	void addStructure(const Unit *unit);
	size_t getWorkerCount();
	std::set<std::pair<const Unit *, std::set<const Unit *>>> getGasWorkers();
	void addGas(const Unit *gas);
	void checkGas(const Unit *gas);
	const Unit *getBuilder();
	void addNewUnit(const Unit *unit);
	void returnToMining(const Unit *unit);
};

