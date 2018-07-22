#include "ProductionQueue.h"
#include "iostream"


ProductionQueue::ProductionQueue()
{

}

void ProductionQueue::initialiseQueue()
{
	productionQueue.push_back(ABILITY_ID::BUILD_PYLON);
	productionQueue.push_back(ABILITY_ID::BUILD_GATEWAY);
	productionQueue.push_back(ABILITY_ID::BUILD_ASSIMILATOR);
	productionQueue.push_back(ABILITY_ID::BUILD_GATEWAY);
	productionQueue.push_back(ABILITY_ID::BUILD_CYBERNETICSCORE);
	productionQueue.push_back(ABILITY_ID::BUILD_ASSIMILATOR);
	productionQueue.push_back(ABILITY_ID::BUILD_GATEWAY);
	productionQueue.push_back(ABILITY_ID::BUILD_PYLON);
	productionQueue.push_back(ABILITY_ID::BUILD_TWILIGHTCOUNCIL);
	productionQueue.push_back(ABILITY_ID::BUILD_PYLON);
	productionQueue.push_back(ABILITY_ID::BUILD_PYLON);
	productionQueue.push_back(ABILITY_ID::RESEARCH_BLINK);
	productionQueue.push_back(ABILITY_ID::BUILD_GATEWAY);
	productionQueue.push_back(ABILITY_ID::BUILD_PYLON);
}

void ProductionQueue::removeItem()
{
	if (productionQueue.empty())
	{
		std::cerr << "queue empty" << std::endl;
		return;
	}
	else
	{
		std::cerr << "removing " << productionQueue.front() << std::endl;
		productionQueue.erase(productionQueue.begin());
	}
}

AbilityID ProductionQueue::getNextItem()
{
	if (productionQueue.empty())
	{
		std::cerr << "queue empty" << std::endl;
		//return UNIT_TYPEID::PROTOSS_PYLON;
		return NULL;
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
	//productionQueue.push_back(ABILITY_ID::BUILD_PYLON);
}