#include "ProductionQueue.h"
#include "iostream"
#include "Blinkerbot.h"


ProductionQueue::ProductionQueue(BlinkerBot & bot): blinkerBot(bot)
{

}

void ProductionQueue::initialiseQueue()
{

	//3gate blink
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_TWILIGHTCOUNCIL));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_BLINK));

	/*
	//3 gate
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	*/

	/*
	//2 gate blink expand
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_TWILIGHTCOUNCIL));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_BLINK));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_NEXUS));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	*/

	/*
	//nexus first into 4gate blink	
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_NEXUS));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_TWILIGHTCOUNCIL));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_BLINK));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	*/

	/*
	//nexus first 6 gate
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_NEXUS));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	*/

	/*
	//super greed
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_NEXUS));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_NEXUS));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	*/
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
		return BuildOrderItem(ABILITY_ID::INVALID);
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
			if (i % 3 == 0)
			{
				productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
			}
			productionQueue.push_back(BuildOrderItem(item.first));
		}
	}
	printDebug();
}

void ProductionQueue::addItemHighPriority(AbilityID type)
{
	productionQueue.insert(productionQueue.begin(), BuildOrderItem(type));
	printDebug();
}

void ProductionQueue::addItemLowPriority(AbilityID type)
{
	productionQueue.push_back(BuildOrderItem(type));
	printDebug();
}

BuildOrderItem::BuildOrderItem(): item(NULL)
{
}

BuildOrderItem::BuildOrderItem(sc2::AbilityID c_item): item(c_item)
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

//checks if there is a unit of the type specified in the current queue
bool ProductionQueue::includes(AbilityID unit)
{
	for (auto item : productionQueue)
	{
		if (item.item == unit)
		{
			return true;
		}
	}
	return false;
}