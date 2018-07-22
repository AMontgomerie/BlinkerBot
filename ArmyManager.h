#pragma once
#include <iostream>
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"

using namespace sc2;

class BlinkerBot;

class ArmyManager
{
	BlinkerBot & blinkerBot;

	std::set<const Unit *> army;
public:
	ArmyManager(BlinkerBot & bot);
	~ArmyManager();
	void onStep();
	void addUnit(const Unit *unit);
};

