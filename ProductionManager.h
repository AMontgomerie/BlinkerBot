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
enum ArmyStatus;

class ProductionManager
{
	const uint32_t DEADLOCK = 700;
	const float NEARBYPYLONDIST = 5.0f;

	struct Warpgate {
		const Unit *warpgate;
		uint32_t lastWarpInLoop;
		UnitTypeID lastTrainedType;

		Warpgate(const Unit *newWarpgate, uint32_t currentLoop, UnitTypeID unitType): 
			warpgate(newWarpgate), lastWarpInLoop(currentLoop), lastTrainedType(unitType){};
	};

	BlinkerBot & blinkerBot;
	ProductionQueue productionQueue;
	BaseManager baseManager;
	WorkerManager workerManager;
	BuildOrderManager buildOrderManager;

	Race enemyRace;
	ArmyStatus armyStatus;
	Point2D nextPylonLocation;
	Point2D rallyPoint;
	Point2D forwardPylonPoint;
	bool enemyHasCloak;
	bool reactedToRush;
	bool beingRushed;
	bool lingSpeed;
	bool massLing;
	int lastProductionFrame;
	const Unit *forwardPylon;
	const Unit *lastUsedWarpGate;

	std::set<const Unit *> assimilators;
	std::set<const Unit *> dts;
	std::set<const Unit *> enemyBases;
	std::set<const Unit *> miningBases;
	std::set<const Unit *> observers;
	std::set<const Unit *> sentries;
	std::set<const Unit *> shieldBatteries;
	std::set<const Unit *> hts;
	std::set<const Unit *> photonCannons;
	std::set<const Unit *> pylons;
	std::set<const Unit *> structures;
	std::vector<Warpgate> warpGates;

public:
	ProductionManager(BlinkerBot & bot);
	void addEnemyBase(const Unit *unit);
	void addNewUnit(const Unit *unit);
	void addStructure(const Unit *unit);
	void checkMineralVisibility();
	void darkShrine(bool signal);
	Point2D getDefensivePosition();
	const Unit *getClosestPylon(Point2D point);
	const Unit *getClosestPylonToEnemyBase();
	void initialise();
	void onStep();
	void onUnitDestroyed(const Unit *unit);
	void onUpgradeComplete(UpgradeID upgrade);
	void receiveArmyStatus(ArmyStatus status);
	void receiveCloakSignal(bool signal);
	void receiveLingSpeedSignal(bool signal);
	void receiveMassLingSignal(bool signal);
	void receiveRushSignal(bool signal);
	void removeEnemyBase(const Unit *unit);
	void removeEnemyProxy(const Unit *unit);
	void setRallyPoint(Point2D point);
	const Unit *accidentalWallIn();

private:
	void breakDeadlock();
	void build(BuildOrderItem item);
	void buildStructure(AbilityID structureToBuild);
	void buildStructure(AbilityID structureToBuild, Point2D target);
	void buildStructure(const Unit *builder, AbilityID structureToBuild, Point2D target);
	int calculateMiningBases();
	float calculateSupplyCapacity();
	bool canAfford(UnitTypeID unit);
	bool canAfford(UpgradeID upgrade);
	bool canAffordGas(UnitTypeID unit);
	void checkSupply();
	int checkPriority(ABILITY_ID ability);
	void chronoBoost();
	bool cleanUp();
	bool completedStructureExists(UnitTypeID structure);
	void expand();
	const Unit *baseNeedsNearbyPylon();
	std::vector<Point2D> getBuildGrid(Point2D centre);
	const Unit *getClosestBaseToEnemy();
	const Unit *getClosestEnemyBase(Point2D point);
	std::set<std::pair<AbilityID, const Unit *>> getCurrentlyInProduction();
	const Unit *getHighestPriorityInProduction(std::set<std::pair<AbilityID, const Unit *>> inProduction);
	const Unit *getIdleWarpgate();
	const Unit *getLeastArtosisPylon();
	bool isBlocking(AbilityID ability);
	void locateForwardPylonPoint();
	void printDebug();
	void printBuildGrid(std::vector<Point2D> buildGrid);
	void produce(BuildOrderItem nextBuildOrderItem);
	void research(BuildOrderItem item);
	void setNextPylonLocation();
	bool structureExists(UnitTypeID structure);
	void train(BuildOrderItem item);
	void trainColossus();
	void trainHighTemplar();
	void trainSentries();
	void trainUnits();
	void trainVoidray();
	void trainWarp();
	void upgrade();
};

