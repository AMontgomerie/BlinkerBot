#include "ProductionQueue.h"
#include "iostream"
#include "Blinkerbot.h"


ProductionQueue::ProductionQueue(BlinkerBot & bot): blinkerBot(bot)
{

}

void ProductionQueue::initialiseQueue()
{
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_TWILIGHTCOUNCIL, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_BLINK, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_NEXUS, Natural));
}

void ProductionQueue::removeItem()
{
	printDebug();
	if (productionQueue.empty())
	{
		//std::cerr << "queue empty" << std::endl;
		return;
	}
	else
	{
		//std::cerr << "removing " << productionQueue.front() << std::endl;
		productionQueue.erase(productionQueue.begin());
	}
}

BuildOrderItem ProductionQueue::getNextItem()
{
	if (productionQueue.empty())
	{
		return BuildOrderItem(ABILITY_ID::BUILD_PYLON, Nowhere);
	}
	else 
	{
		//std::cerr << "next item: " << productionQueue.front() << std::endl;
		return productionQueue.front();
	}
}

ProductionQueue::~ProductionQueue()
{
}

//currently just returns something unbuildable, but plan to create some sort of build order generator here
void ProductionQueue::generateMoreItems(std::set<std::pair<AbilityID, int>> buildOrderGoal)
{
	for (auto item : buildOrderGoal)
	{
		for (int i = 0; i != item.second; i++)
		{
			productionQueue.push_back(BuildOrderItem(item.first, Main));
		}
	}
	printDebug();
}

void ProductionQueue::addItemHighPriority(AbilityID type, Location location)
{
	productionQueue.insert(productionQueue.begin(), BuildOrderItem(type, location));
	printDebug();
}

void ProductionQueue::addItemLowPriority(AbilityID type, Location location)
{
	productionQueue.push_back(BuildOrderItem(type, location));
	printDebug();
}

BuildOrderItem::BuildOrderItem(): item(NULL), location(Main)
{
}

BuildOrderItem::BuildOrderItem(sc2::AbilityID c_item, Location c_location): item(c_item), location(c_location)
{
}

void ProductionQueue::printDebug()
{
	for (auto item : productionQueue)
	{
		blinkerBot.Debug()->DebugTextOut(AbilityTypeToName(item.item));
	}
	blinkerBot.Debug()->SendDebug();
}