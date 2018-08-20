#pragma once

#include <iostream>
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"
#include "ProductionQueue.h"
#include "BaseManager.h"
#include "UnitData.h"
#include "WorkerManager.h"
#include "BuildOrderManager.h"

using namespace sc2;

class BlinkerBot;

class ProductionManager
{
	const uint32_t DEADLOCK = 1000;

	BlinkerBot & blinkerBot;
	ProductionQueue productionQueue;
	BaseManager baseManager;
	WorkerManager workerManager;
	BuildOrderManager buildOrderManager;

	std::set<const Unit *> structures;
	std::set<const Unit *> pylons;
	std::set<const Unit *> enemyBases;
	std::set<const Unit*> miningBases;
	Point2D nextPylonLocation;
	const Unit *forwardPylon;
	Point2D rallyPoint;
	Point2D forwardPylonPoint;
	bool attacking;
	bool enemyHasCloak;
	int lastProductionFrame;

	void buildStructure(AbilityID structureToBuild);
	void buildStructure(AbilityID structureToBuild, Point2D target);
	void buildStructure(const Unit *builder, AbilityID structureToBuild, Point2D target);
	void produce(BuildOrderItem nextBuildOrderItem);
	void trainUnits();
	void locateForwardPylonPoint();
	void checkSupply();
	void expand();
	void chronoBoost();
	std::set<std::pair<AbilityID, const Unit *>> getCurrentlyInProduction();
	const Unit *getHighestPriorityInProduction(std::set<std::pair<AbilityID, const Unit *>> inProduction);
	int checkPriority(ABILITY_ID ability);
	int calculateMiningBases();
	void setNextPylonLocation();
	const Unit *getLeastArtosisPylon();
	void printDebug();
	std::vector<Point2D> getBuildGrid(Point2D centre);
	void printBuildGrid(std::vector<Point2D> buildGrid);
	void research(BuildOrderItem item);
	void train(BuildOrderItem item);
	void build(BuildOrderItem item);
	const Unit *getClosestEnemyBase(Point2D point);
	void breakDeadlock();
	float calculateSupplyCapacity();
public:
	ProductionManager(BlinkerBot & bot);
	~ProductionManager();
	void initialise();
	void onStep();
	void onUnitDestroyed(const Unit *unit);
	void addStructure(const Unit *unit);
	void addNewUnit(const Unit *unit);
	void receiveAttackSignal(bool attack);
	void addEnemyBase(const Unit *unit);
	void removeEnemyBase(const Unit *unit);
	void receiveCloakSignal(bool signal);
	void checkMineralVisibility();
	void onUpgradeComplete(UpgradeID upgrade);
	const Unit *getClosestPylonToEnemyBase();
	const Unit *getClosestPylon(Point2D point);
	void setRallyPoint(Point2D point);
	void returnToMining(const Unit *unit);
};

