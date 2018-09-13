#include "Blinkerbot.h"
#include "sc2api/sc2_api.h"

using namespace sc2;

BlinkerBot::BlinkerBot(): productionManager(*this), armyManager(*this)
{
}


void BlinkerBot::OnGameStart()
{
	rallyPoint = Observation()->GetStartLocation();
	productionManager.initialise();
	armyManager.initialise();
}

void BlinkerBot::OnStep()
{
	/*
	//timer stuff
	std::clock_t start;
	double duration;
	start = std::clock();
	//timer stuff
	*/

	productionManager.onStep();
	productionManager.receiveArmyStatus(armyManager.getArmyStatus());
	productionManager.receiveCloakSignal(armyManager.detectionRequired());
	productionManager.receiveRushSignal(armyManager.rushDetected());
	armyManager.onStep();


	//determine the location of our rally point
	ArmyStatus currentStatus = armyManager.getArmyStatus();
	const Unit *aggressivePylon = productionManager.getClosestPylonToEnemyBase();
	Point2D defensivePosition = productionManager.getDefensivePosition();
	const Unit *threatenedStructure = armyManager.underAttack();

	//if we are attacking then let's use whichever pylon is closest to the enemy
	if (currentStatus == Attack && aggressivePylon && !threatenedStructure)
	{
		rallyPoint = aggressivePylon->pos;
	}
	//if we aren't attacking then we want to rally to nearby one of our bases
	else if (currentStatus != Attack  && !threatenedStructure)
	{
		rallyPoint = defensivePosition;
	}
	//if we're under attack then rally towards that location
	else if (threatenedStructure)
	{
		rallyPoint = productionManager.getClosestPylon(threatenedStructure->pos)->pos;
	}
	else
	{
		rallyPoint = Observation()->GetStartLocation();
	}
	productionManager.setRallyPoint(rallyPoint);
	armyManager.setRallyPoint(rallyPoint);

	//when behind, dark shrine!
	productionManager.darkShrine(armyManager.behind());

	/*
	//timer stuff
	duration = (std::clock() - start) / (double)CLOCKS_PER_SEC * 1000;
	if (duration > 20)
	{
		std::cout << "frame duration: " << duration << '\n';
	}
	//timer stuff
	*/
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
		if (UnitData::isStructure(unit))
		{
			armyManager.removeEnemyStructure(unit);
			if (UnitData::isTownHall(unit))
			{
				productionManager.removeEnemyBase(unit);
			}
		}
		else
		{
			armyManager.removeEnemyUnit(unit);
		}
	}
}
void BlinkerBot::OnUnitEnterVision(const sc2::Unit *unit)
{
	if (UnitData::isStructure(unit))
	{
		armyManager.addEnemyStructure(unit);
		if (UnitData::isTownHall(unit))
		{
			productionManager.addEnemyBase(unit);
		}
	}
	else
	{
		armyManager.addEnemyUnit(unit);
	}
}

void BlinkerBot::OnBuildingConstructionComplete(const sc2::Unit* unit)
{
	productionManager.addStructure(unit);
}

void BlinkerBot::OnUnitCreated(const sc2::Unit *unit)
{
	productionManager.addNewUnit(unit);

	if (unit->unit_type == UNIT_TYPEID::PROTOSS_ZEALOT || 
			 unit->unit_type == UNIT_TYPEID::PROTOSS_STALKER || 
			 unit->unit_type == UNIT_TYPEID::PROTOSS_COLOSSUS || 
			 unit->unit_type == UNIT_TYPEID::PROTOSS_DARKTEMPLAR ||
			 unit->unit_type == UNIT_TYPEID::PROTOSS_OBSERVER ||
			 unit->unit_type == UNIT_TYPEID::PROTOSS_IMMORTAL)
	{
		armyManager.addUnit(unit);
	}
}

void BlinkerBot::OnUnitIdle(const sc2::Unit *unit)
{

}

void BlinkerBot::OnUpgradeCompleted(UpgradeID upgrade)
{
	productionManager.onUpgradeComplete(upgrade);
	if (upgrade == UPGRADE_ID::WARPGATERESEARCH)
	{
		armyManager.warpgateComplete();
	}
}

