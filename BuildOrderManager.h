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
	~ProductionGoal();
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
	bool enemyHasCloak;
	bool miningOut;

private:
	void initialiseKeyTechs();
	std::set<const Unit *> getBases();
	void updateStructureCounts();
	void addCompletedTech(UpgradeID upgrade);
	void removeKeyTech(UpgradeID upgrade);
	void removeKeyTech(AbilityID ability);
	bool inProgress(AbilityID ability);
	AbilityID getNextTech();
public:
	bool alreadyResearched(AbilityID ability);
	std::vector<ProductionGoal> generateGoal();
	void onUpgradeComplete(UpgradeID upgrade);
	BuildOrderManager(BlinkerBot & bot);
	~BuildOrderManager();
	void receiveCloakSignal(bool signal);
	void receiveMiningOutSignal(bool signal);
	bool isKeyTech(AbilityID ability);
};

