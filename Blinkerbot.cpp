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
	productionManager.receiveMassLingSignal(armyManager.massLings());
	productionManager.receiveLingSpeedSignal(armyManager.lingSpeed());
	armyManager.breakWall(productionManager.accidentalWallIn());
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
		const Unit *closestPylon = productionManager.getClosestPylon(threatenedStructure->pos);
		if (closestPylon)
		{
			rallyPoint = closestPylon->pos;
		}
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
		if (UnitData::isStructure(unit))
		{
			armyManager.removeStructure(unit);
		}
	}
	else
	{
		if (UnitData::isStructure(unit))
		{
			armyManager.removeEnemyStructure(unit);
			productionManager.removeEnemyProxy(unit);
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
		productionManager.addEnemyBase(unit);
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
	armyManager.addUnit(unit);
	if (UnitData::isStructure(unit))
	{
		armyManager.addStructure(unit);
	}
}

void BlinkerBot::OnUnitIdle(const sc2::Unit *unit)
{

}

void BlinkerBot::OnUpgradeCompleted(UpgradeID upgrade)
{
	productionManager.onUpgradeComplete(upgrade);
	armyManager.onUpgradeComplete(upgrade);
}

