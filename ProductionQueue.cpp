#include "ProductionQueue.h"
#include "iostream"


ProductionQueue::ProductionQueue()
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
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_WARPGATE, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_TWILIGHTCOUNCIL, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::RESEARCH_BLINK, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_GATEWAY, Main));
	productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON, Main));
}

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
}

BuildOrderItem ProductionQueue::getNextItem()
{
	if (productionQueue.empty())
	{
		std::cerr << "queue empty, adding extra pylon" << std::endl;
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

void ProductionQueue::generateMoreItems()
{
	//std::cerr << "generating" << std::endl;
	//productionQueue.push_back(BuildOrderItem(ABILITY_ID::BUILD_PYLON, Nowhere));
}

void ProductionQueue::addItemHighPriority(AbilityID type, Location location)
{
	productionQueue.insert(productionQueue.begin(), BuildOrderItem(type, location));
}

void ProductionQueue::addItemLowPriority(AbilityID type, Location location)
{
	productionQueue.push_back(BuildOrderItem(type, location));
}

BuildOrderItem::BuildOrderItem(): item(NULL), location(Main)
{
}

BuildOrderItem::BuildOrderItem(sc2::AbilityID c_item, Location c_location): item(c_item), location(c_location)
{
}