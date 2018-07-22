#include "ArmyManager.h"
#include "Blinkerbot.h"


ArmyManager::ArmyManager(BlinkerBot & bot) : blinkerBot(bot)
{
}


ArmyManager::~ArmyManager()
{
}

void ArmyManager::onStep()
{
	if (army.size() >= 12)
	{
		for (auto unit : army)
		{
			blinkerBot.Actions()->UnitCommand(unit, ABILITY_ID::ATTACK, blinkerBot.Observation()->GetGameInfo().enemy_start_locations.front());
		}
	}
}

void ArmyManager::addUnit(const Unit *unit)
{
	army.insert(unit);
}
