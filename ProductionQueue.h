#pragma once

#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"

using namespace sc2;

class BlinkerBot;

class BuildOrderItem
{
public:
	sc2::AbilityID item;

	BuildOrderItem();
	BuildOrderItem(sc2::AbilityID c_item);
};

class ProductionQueue
{
	BlinkerBot & blinkerBot;
	std::vector<BuildOrderItem> productionQueue;

	void printDebug();
public:
	ProductionQueue(BlinkerBot & bot);
	~ProductionQueue();
	void removeItem();
	void initialiseQueue();
	BuildOrderItem getNextItem();
	void generateMoreItems(std::set<std::pair<AbilityID, int>> buildOrderGoal);
	void addItemHighPriority(AbilityID type);
	void addItemLowPriority(AbilityID type);
	bool includes(AbilityID unit);
};
