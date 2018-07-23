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
	std::set<const Unit *> pylons;
	const Unit *forwardPylon;
	Point2D rallyPoint;
	Point2D forwardPylonPoint;

	void buildStructure(AbilityID structureToBuild, Location location);
	void buildStructure(AbilityID structureToBuild, Point2D target);
	void buildStructure(const Unit *builder, AbilityID structureToBuild, Point2D target);
	const Unit *getClosestGas(const Unit *unit);
	const Unit *getClosestGas(Point3D point);
	void findGeysers();
	void produce(BuildOrderItem nextBuildOrderItem);
	void trainUnits();
	void locateForwardPylonPoint();
	const Unit *getClosestPylon(Point2D point);
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
	void receiveAttackSignal(bool attack);
	Point2D getRallyPoint();
};

