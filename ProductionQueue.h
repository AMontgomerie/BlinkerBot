#pragma once

#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"

using namespace sc2;

class ProductionQueue
{
	std::vector<AbilityID> productionQueue;

public:
	ProductionQueue();
	~ProductionQueue();
	void removeItem();
	void initialiseQueue();
	AbilityID getNextItem();
	void generateMoreItems();
};

