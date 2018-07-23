#pragma once

#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"

using namespace sc2;

enum Location { Proxy, Main, Natural, Third , Nowhere };

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
	std::vector<BuildOrderItem> productionQueue;

public:
	ProductionQueue();
	~ProductionQueue();
	void removeItem();
	void initialiseQueue();
	BuildOrderItem getNextItem();
	void generateMoreItems();
	void addItemHighPriority(AbilityID type, Location location);
	void addItemLowPriority(AbilityID type, Location location);
};
