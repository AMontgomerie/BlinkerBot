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
	const int WAITTIME = 500;

	struct ArmyUnit
	{
		const Unit *unit;
		ArmyStatus status;
		ArmyUnit(const Unit *armyUnit, ArmyStatus unitStatus): unit(armyUnit), status(unitStatus){};
	};

	struct ZerglingTimer
	{
		const Unit *zergling;
		float startTime;
		float endTime;
		bool onCreep;
		Point2D startPosition;
		ZerglingTimer(const Unit *ling, float sTime, float eTime, Point2D sPos, bool creep):
			zergling(ling), startTime(sTime), endTime(eTime), startPosition(sPos), onCreep(creep){};
	};

	Race enemyRace;
	bool beingRushed;
	bool proxy;
	int regroupStarted;
	bool regroupComplete;
	bool zerglingSpeed;
	bool warpgateTech;
	bool blinkTech;
	float currentArmyValue;
	float currentEnemyArmyValue;
	ArmyStatus currentStatus;
	Point2D rallyPoint;
	ZerglingTimer zerglingTimer;

	std::vector<ArmyUnit> army;
	std::vector<Point2D> unexploredEnemyStartLocations;
	std::set<const Unit *> darkTemplars;
	std::set<const Unit *> highTemplars;
	std::set<const Unit *> enemyArmy;
	std::set<const Unit *> enemyStructures;
	std::set<const Unit *> observers;
	std::set<const Unit *> photonCannons;
	std::set<const Unit *> bases;


public:
	ArmyManager(BlinkerBot & bot);
	void addEnemyStructure(const Unit *structure);
	void addEnemyUnit(const Unit *unit);
	void addUnit(const Unit *unit);
	bool behind();
	bool detectionRequired();
	ArmyStatus getArmyStatus();
	void initialise();
	void onStep();
	void onUpgradeComplete(UpgradeID upgrade);
	void removeEnemyUnit(const Unit *unit);
	void removeEnemyStructure(const Unit *structure);
	void removeUnit(const Unit *unit);
	bool rushDetected();
	void setRallyPoint(Point2D point);
	const Unit *underAttack();

private:
	void activatePrismaticAlignment(const Unit *voidray);
	bool aggressiveBlink(Point2D target);
	void attack();
	void attack(Point2D target);
	float averageUnitDistanceToEnemyBase();
	bool blink(const Unit *unit);
	int calculateEnemyStaticDefence();
	float calculateEnemyStaticDefenceInRadius(Point2D centre);
	float calculateSupply(std::set<const Unit *> army);
	float calculateSupply(std::vector<ArmyUnit> army);
	float calculateSupplyAndWorkers(std::set<const Unit *> army);
	float calculateSupplyAndWorkersInRadius(Point2D centre, std::set<const Unit *> army);
	float calculateSupplyInRadius(Point2D centre, std::set<const Unit *> army);
	float calculateSupplyInRadius(Point2D centre, std::vector<ArmyUnit> army);
	bool canAttack();
	void checkForZerglingSpeed();
	void darkTemplarHarass();
	void defend(Point2D threatened);
	bool escapeAOE(ArmyUnit armyUnit);
	void feedback();
	Point2D findAttackTarget(const Unit *unit);
	Point2D getAOETarget(std::set<const Unit *> unitsInRange, float radius);
	const Unit *getClosestBase(const Unit *unit);
	const Unit *getClosestBase(Point2D point);
	const Unit *getClosestEnemy(const Unit *ourUnit);
	const Unit *getClosestEnemy(Point2D point);
	const Unit *getClosestEnemyBase(const Unit *ourUnit);
	const Unit *getClosestEnemyBaseWithoutDetection(const Unit *unit);
	const Unit *getClosestEnemyFlyer(Point2D point);
	Point2D getRetreatPoint(const Unit *unit);
	bool includes(UnitTypeID unitType);
	bool inRange(const Unit *attacker, const Unit *target);
	bool inRange(const Unit *attacker, Point2D target);
	bool isUnderHostileSpell(Point2D target);
	bool isUnderPsistorm(Point2D target);
	bool kite(ArmyUnit armyUnit);
	void moveObservers();
	bool outranges(const Unit *attacker, const Unit *target);
	void printDebug();
	void psistorm();
	bool regroup();
	void retreat();
	bool shieldsCritical(const Unit *unit, const Unit *attacker);
	void updateArmyValues();
};

