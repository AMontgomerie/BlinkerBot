#include "Blinkerbot.h"
#include "sc2api/sc2_api.h"

using namespace sc2;

BlinkerBot::BlinkerBot(): productionManager(*this), armyManager(*this)
{
}


void BlinkerBot::OnGameStart()
{
	productionManager.initialise();
	for (auto unit : Observation()->GetUnits())
	{
		if (unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE)
		{
			productionManager.addWorker(unit);
		}
		else if (unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
		{
			productionManager.addStructure(unit);
		}
	}
}

void BlinkerBot::OnStep()
{
	productionManager.onStep();
	armyManager.onStep();
	productionManager.receiveAttackSignal(armyManager.sendAttackSignal());
	armyManager.receiveRallyPoint(productionManager.getRallyPoint());
}

void BlinkerBot::OnUnitDestroyed(const sc2::Unit *unit)
{
	if (UnitData::isOurs(unit))
	{
		productionManager.onUnitDestroyed(unit);
		armyManager.removeUnit(unit);
	}
	else
	{
		armyManager.removeEnemyUnit(unit);
	}
}
void BlinkerBot::OnUnitEnterVision(const sc2::Unit *unit)
{
	armyManager.addEnemyUnit(unit);
}

void BlinkerBot::OnBuildingConstructionComplete(const sc2::Unit* unit)
{
	productionManager.addStructure(unit);
	if (unit->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR)
	{
		productionManager.addGas(unit);
	}
}

void BlinkerBot::OnUnitCreated(const sc2::Unit *unit)
{
	if ((unit->unit_type == UNIT_TYPEID::PROTOSS_ZEALOT) || (unit->unit_type == UNIT_TYPEID::PROTOSS_STALKER))
	{
		armyManager.addUnit(unit);
	}
	else
	{
		productionManager.addNewUnit(unit);
	}
	
}

void BlinkerBot::OnUnitIdle(const sc2::Unit *unit)
{
	if (unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE)
	{
		productionManager.returnToMining(unit);
	}
}

