#include "ProductionQueue.h"
#include "iostream"
#include "Blinkerbot.h"

ProductionQueue::ProductionQueue(BlinkerBot & bot): blinkerBot(bot), enemyRace(Race::Random){}

/*
adds an initial queue of BuildOrderItems (representing our opening build order) to the production queue.
*/
void ProductionQueue::initialiseQueue()
{
	switch (enemyRace)
	{
	case Race::Terran:
		loadPvTBuildOrder();
		break;
	case Race::Zerg:
		loadPvZBuildOrder();
		break;
	case Race::Protoss:
	case Race::Random:
	default:
		loadPvPBuildOrder();
		break;
	}

	/*
	//sentries
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	*/

	/*
	//voidray
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_STARGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::TRAIN_VOIDRAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::TRAIN_VOIDRAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::TRAIN_VOIDRAY));
	*/

	/*
	//1 base storm
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_TWILIGHTCOUNCIL));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_TEMPLARARCHIVE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_PSISTORM));
	*/

	/*
	//dt rush
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_TWILIGHTCOUNCIL));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_DARKSHRINE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::TRAINWARP_DARKTEMPLAR));
	*/

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
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
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

/*
remove the item from the front of the queue
*/
void ProductionQueue::removeItem()
{
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
	printDebug();
}

/*
return the item at the front of the queue
*/
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

/*
takes a build order goal passed from the production manager and translates it into BuildOrderItems that are added to the queue.
Build order goals are sets of pairs containing AbilityIDs (units to be made) and ints (the quantity of the unit to be made).
*/
void ProductionQueue::generateMoreItems(std::vector<ProductionGoal> buildOrderGoal)
{
	int count = 0;
	for (auto item : buildOrderGoal)
	{
		for (int i = 0; i != item.quantity; i++)
		{
			productionQueue.push_back(BuildOrderItem(item.type));
			count++;
			if (count % 3 == 2)
			{
				productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
			}
		}
	}
	printDebug();
}

/*
add something to the front of the queue
*/
void ProductionQueue::addItemHighPriority(AbilityID type)
{
	productionQueue.insert(productionQueue.begin(), BuildOrderItem(type));
	printDebug();
}

/*
add something to the back of the queue
*/
void ProductionQueue::addItemLowPriority(AbilityID type)
{
	productionQueue.push_back(BuildOrderItem(type));
	printDebug();
}

BuildOrderItem::BuildOrderItem(): item(NULL){}

BuildOrderItem::BuildOrderItem(sc2::AbilityID c_item): item(c_item){}

/*
prints the current production queue in the top left of the screen
*/
void ProductionQueue::printDebug()
{
	/*
	for (auto item : productionQueue)
	{
		blinkerBot.Debug()->DebugTextOut(AbilityTypeToName(item.item));
	}
	blinkerBot.Debug()->SendDebug();
	*/
}

/*
checks if there is a unit of the type specified in the current queue
*/
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

/*
empties the production queue
*/
void ProductionQueue::clearQueue()
{
	productionQueue.clear();
	printDebug();
}

/*
assign a value to enemyRace
*/
void ProductionQueue::setEnemyRace(Race race)
{
	enemyRace = race;
}

void ProductionQueue::loadPvPBuildOrder()
{
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_TWILIGHTCOUNCIL));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_BLINK));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ROBOTICSFACILITY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::TRAIN_IMMORTAL));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
}

void ProductionQueue::loadPvTBuildOrder()
{
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_TWILIGHTCOUNCIL));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_BLINK));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_NEXUS));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_FORGE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PHOTONCANNON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PHOTONCANNON));

	/*
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_TWILIGHTCOUNCIL));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_BLINK));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_NEXUS));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ROBOTICSFACILITY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_TEMPLARARCHIVE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_FORGE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_PSISTORM));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PHOTONCANNON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PHOTONCANNON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	*/
}

void ProductionQueue::loadPvZBuildOrder()
{
	/*
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_NEXUS));
	*/

	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_NEXUS));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_CYBERNETICSCORE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_TWILIGHTCOUNCIL));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_FORGE));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_BLINK));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ASSIMILATOR));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_ROBOTICSFACILITY));

}

void ProductionQueue::reorderQueue()
{
	
}

/*
finds an element from the production queue and removes it
*/
bool ProductionQueue::findAndRemoveItem(AbilityID ability)
{
	if (productionQueue.empty())
	{
		return false;
	}

	for (std::vector<BuildOrderItem>::iterator item = productionQueue.begin(); item != productionQueue.end();)
	{
		if ((*item).item == ability)
		{
			item = productionQueue.erase(item);
			return true;
		}
		else
		{
			++item;
		}
	}
	return false;
}