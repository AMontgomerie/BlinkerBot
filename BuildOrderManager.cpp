#include "BuildOrderManager.h"
#include "Blinkerbot.h"

BuildOrderManager::BuildOrderManager(BlinkerBot & bot) : blinkerBot(bot), enemyHasCloak(false),
currentBases(0), currentGases(0), currentProductionFacilities(0), currentCannons(0)
{
	initialiseKeyTechs();
}

BuildOrderManager::~BuildOrderManager()
{
}

ProductionGoal::ProductionGoal(AbilityID type, int quantity)
{
	this->type = type;
	this->quantity = quantity;
}

ProductionGoal::~ProductionGoal()
{
}

/*
count the numbers of various types of structures under our control
*/
void BuildOrderManager::updateStructureCounts()
{
	int bases = 0;
	int productionFacilities = 0;
	int gases = 0;
	int cannons = 0;

	for (auto structure : blinkerBot.Observation()->GetUnits())
	{
		if (UnitData::isOurs(structure) && UnitData::isStructure(structure))
		{
			if (structure->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
			{
				bases++;
			}
			else if (structure->unit_type == UNIT_TYPEID::PROTOSS_GATEWAY ||
				structure->unit_type == UNIT_TYPEID::PROTOSS_WARPGATE ||
				structure->unit_type == UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY)
			{
				productionFacilities++;
			}
			else if (structure->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR)
			{
				gases++;
			}
			else if (structure->unit_type == UNIT_TYPEID::PROTOSS_PHOTONCANNON)
			{
				cannons++;
			}
		}
	}

	currentBases = bases;
	currentGases = gases;
	currentProductionFacilities = productionFacilities;
	currentCannons = cannons;
}

/*
returns a set of AbilityID and int pairs representing things we want to produce and the quantity of each thing we want
*/
std::vector<ProductionGoal> BuildOrderManager::generateGoal()
{
	std::vector<ProductionGoal> buildOrderGoal;

	//first let's count the number of things we currently have
	updateStructureCounts();

	AbilityID tech = getNextTech();
	if (tech != ABILITY_ID::INVALID)
	{
		buildOrderGoal.push_back(ProductionGoal(tech, 1));
	}

	//if the enemy has cloaked units, we want to make sure we have cannons at each base
	if (enemyHasCloak || currentBases > 1)
	{
		int extraCannons = currentBases - currentCannons;
		if (extraCannons > 0)
		{
			buildOrderGoal.push_back(ProductionGoal(ABILITY_ID::BUILD_PHOTONCANNON, extraCannons));
		}
	}

	//add extra gateways at a 3:1 gateway to base ratio as our economy improves
	int extraProductionFacilities = (currentBases * 2) - currentProductionFacilities;
	if (extraProductionFacilities > 0)
	{

		//we don't wanna add too many extra production facilities at one time as this will slow down our production
		if (extraProductionFacilities > 2)
		{
			buildOrderGoal.push_back(ProductionGoal(ABILITY_ID::BUILD_GATEWAY, 2));
		}
		else
		{
			buildOrderGoal.push_back(ProductionGoal(ABILITY_ID::BUILD_GATEWAY, extraProductionFacilities));
		}
	}

	//if we've alreadygot plenty of production facilities then we want to think about expanding
	if (extraProductionFacilities < 1 || miningOut)
	{
		buildOrderGoal.push_back(ProductionGoal(ABILITY_ID::BUILD_NEXUS, 1));
	}

	//count bases excluding those that only recently started being produced (we don't wanna take gas too early)
	int readyToMineBases = 0;
	for (auto base : getBases())
	{
		if (base->build_progress > 0.8)
		{
			readyToMineBases++;
		}
	}

	//calculate any additional gases we want to take
	int additionalGases = (readyToMineBases * 2) - currentGases;
	if (additionalGases > 0)
	{
		buildOrderGoal.push_back(ProductionGoal(ABILITY_ID::BUILD_ASSIMILATOR, additionalGases));
	}

	if (buildOrderGoal.empty())
	{
		buildOrderGoal.push_back(ProductionGoal(ABILITY_ID::BUILD_PYLON, 1));
	}

	/*
	std::cerr << "current bases: " << currentBases << std::endl;
	std::cerr << "current gateways: " << currentProductionFacilities << std::endl;
	std::cerr << "goal state: " << currentBases << " bases and " << currentProductionFacilities + extraProductionFacilities << " gateways" << std::endl;
	std::cerr << "adding to production queue:" << std::endl;
	for (auto item : buildOrderGoal)
	{
	for (int i = 0; i != item.second; i++)
	{
	std::cerr << AbilityTypeToName(item.first) << std::endl;
	}
	}
	*/

	return buildOrderGoal;
}

/*
called when an upgrade is completed
*/
void BuildOrderManager::onUpgradeComplete(UpgradeID upgrade)
{
	removeKeyTech(upgrade);
}

/*
checks the vector of key techs, removes any that have already been researched, and returns the next item
*/
AbilityID BuildOrderManager::getNextTech()
{
	//check that there are some remaining techs
	if (!keyTechs.empty())
	{
		//check through to see if any techs are already researched (and remove them if they are)
		std::vector<AbilityID>::iterator nextTech = keyTechs.begin();
		while (alreadyResearched(*nextTech) && nextTech != keyTechs.end())
		{
			//std::cerr << "removing already completed tech: " << AbilityTypeToName(*nextTech) << std::endl;
			removeKeyTech(*nextTech++);
		}
		//we might have removed some so make sure we still have some left
		if (!keyTechs.empty())
		{
			//check through to see if any techs are already in progress (and skip them (but don't remove them) if they are)
			std::vector<AbilityID>::iterator nextTech = keyTechs.begin();
			while (inProgress(*nextTech) && nextTech != keyTechs.end())
			{
				//std::cerr << "skipping already started tech: " << AbilityTypeToName(*nextTech) << std::endl;
				nextTech++;
			}
			return *nextTech;
		}
		else
		{
			return ABILITY_ID::INVALID;
		}
	}
	else
	{
		return ABILITY_ID::INVALID;
	}
	
}

//checks our structures to see if we already have this ability in progress
bool BuildOrderManager::inProgress(AbilityID ability)
{
	for (auto unit : blinkerBot.Observation()->GetUnits())
	{
		if (UnitData::isOurs(unit) && UnitData::isStructure(unit))
		{
			for (auto order : unit->orders)
			{
				if (order.ability_id == ability || UnitData::isComparableUpgrade(order.ability_id, ability))
				{
					return true;
				}
			}
		}
	}
	return false;
}

/*
1/2 removes a key tech from the vector
*/
void BuildOrderManager::removeKeyTech(UpgradeID upgrade)
{
	for (std::vector<AbilityID>::iterator tech = keyTechs.begin(); tech != keyTechs.end();)
	{
		if (UnitData::getAbilityID(upgrade) == *tech)
		{
			//std::cerr << "removing a key tech: " << AbilityTypeToName(*tech) << std::endl;
			tech = keyTechs.erase(tech);
		}
		else
		{
			++tech;
		}
	}
}

/*
2/2 removes a key tech from the vector
*/
void BuildOrderManager::removeKeyTech(AbilityID ability)
{
	for (std::vector<AbilityID>::iterator tech = keyTechs.begin(); tech != keyTechs.end();)
	{
		if (ability == *tech)
		{
			//std::cerr << "removing key tech: " << AbilityTypeToName(*tech) << std::endl;
			tech = keyTechs.erase(tech);
		}
		else
		{
			++tech;
		}
	}
}

/*
returns true if a tech has already been researched
*/
bool BuildOrderManager::alreadyResearched(AbilityID ability)
{
	for (auto completed : completedTechs)
	{
		if (UnitData::getAbilityID(completed) == ability)
		{
			return true;
		}
	}
	return false;
}

/*
tells us if the enemy has cloak or not
*/
void BuildOrderManager::receiveCloakSignal(bool signal)
{
	enemyHasCloak = signal;
}

/*
returns a set of our bases
*/
std::set<const Unit *> BuildOrderManager::getBases()
{
	std::set<const Unit *> bases;
	for (auto unit : blinkerBot.Observation()->GetUnits())
	{
		if (UnitData::isOurs(unit) && UnitData::isTownHall(unit))
		{
			bases.insert(unit);
		}
	}
	return bases;
}

/*
tells us when one of our bases is mining out
*/
void BuildOrderManager::receiveMiningOutSignal(bool signal)
{
	miningOut = signal;
}

/*
creates an ordered list of important upgrades that need to be researched throughout the game
*/
void BuildOrderManager::initialiseKeyTechs()
{
	keyTechs.push_back(ABILITY_ID::RESEARCH_WARPGATE);
	keyTechs.push_back(ABILITY_ID::RESEARCH_BLINK);
	keyTechs.push_back(ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL1);
	keyTechs.push_back(ABILITY_ID::RESEARCH_CHARGE);
	keyTechs.push_back(ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL2);
	keyTechs.push_back(ABILITY_ID::RESEARCH_EXTENDEDTHERMALLANCE);
	keyTechs.push_back(ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL3);
	keyTechs.push_back(ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL1);
	keyTechs.push_back(ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL2);
	keyTechs.push_back(ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL3);
	keyTechs.push_back(ABILITY_ID::RESEARCH_PROTOSSSHIELDSLEVEL1);
	keyTechs.push_back(ABILITY_ID::RESEARCH_PROTOSSSHIELDSLEVEL2);
	keyTechs.push_back(ABILITY_ID::RESEARCH_PROTOSSSHIELDSLEVEL3);
}

/*
add an upgrade to the set of completed ones
*/
void BuildOrderManager::addCompletedTech(UpgradeID upgrade)
{
	completedTechs.insert(upgrade);
}