#pragma once

#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"

using namespace sc2;

enum Location { Main, Natural, Third, Proxy, Nowhere };

class BlinkerBot;

class BuildOrderItem
{
public:
	sc2::AbilityID item;
	Location location;

	BuildOrderItem();
	BuildOrderItem(sc2::AbilityID c_item, Location c_location);
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
	void addItemHighPriority(AbilityID type, Location location);
	void addItemLowPriority(AbilityID type, Location location);
	bool includes(AbilityID unit);
};
