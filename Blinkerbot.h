#pragma once

#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"
#include "ProductionManager.h"
#include "ArmyManager.h"
#include <iostream>

using namespace sc2;


class BlinkerBot : public sc2::Agent
{
	ProductionManager productionManager;
	ArmyManager armyManager;

public:
	BlinkerBot();
	virtual void OnGameStart() override;
	virtual void OnStep() override;
    virtual void OnUnitIdle(const Unit *unit) override;
	virtual void OnUnitDestroyed(const Unit *unit) override;
    virtual void OnUnitCreated(const Unit *unit) override;
    virtual void OnUnitEnterVision(const Unit *unit) override;
	virtual void OnBuildingConstructionComplete(const Unit *unit) override;

};

