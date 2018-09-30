#pragma once
#include <iostream>
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"
#include "UnitData.h"

using namespace sc2;

class BlinkerBot;

struct ProductionGoal
{
public:
	AbilityID type;
	int quantity;

	ProductionGoal(AbilityID type, int quantity);
};

class BuildOrderManager
{
	BlinkerBot & blinkerBot;

	std::vector<AbilityID> keyTechs;
	std::set<UpgradeID> completedTechs;
	int currentBases;
	int currentGases;
	int currentProductionFacilities;
	int currentCannons;
	int currentWorkers;
	bool enemyHasCloak;
	bool miningOut;
	Race enemyRace;

private:
	void initialiseKeyTechs();
	std::set<const Unit *> getBases();
	void updateStructureCounts();
	void addCompletedTech(UpgradeID upgrade);
	void removeKeyTech(UpgradeID upgrade);
	void removeKeyTech(AbilityID ability);
	AbilityID getNextTech();
	std::set<AbilityID> getTechsCurrentlyInProduction();
	std::vector<ProductionGoal> prioritiseGoal(std::vector<ProductionGoal> goal);
public:
	bool inProgress(AbilityID ability);
	bool alreadyResearched(AbilityID ability);
	std::vector<ProductionGoal> generateGoal();
	void onUpgradeComplete(UpgradeID upgrade);
	BuildOrderManager(BlinkerBot & bot);
	void receiveCloakSignal(bool signal);
	void receiveMiningOutSignal(bool signal);
	bool isKeyTech(AbilityID ability);
	std::vector<ProductionGoal> generateRushDefenceGoal();
	void setEnemyRace(Race race);
	void initialise();
};

