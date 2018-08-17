#include "UnitData.h"
//#include "Blinkerbot.h"


UnitData::UnitData()
{
}


UnitData::~UnitData()
{
}

bool UnitData::isWorker(const Unit *unit)
{
	if ((unit->unit_type == UNIT_TYPEID::TERRAN_SCV) ||
		(unit->unit_type == UNIT_TYPEID::TERRAN_MULE) ||
		(unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE) ||
		(unit->unit_type == UNIT_TYPEID::ZERG_DRONE))
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool UnitData::isStructure(const Unit *unit)
{
	if ((unit->unit_type == UNIT_TYPEID::TERRAN_ARMORY) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_BARRACKS) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_BARRACKSFLYING) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_BARRACKSREACTOR) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_BARRACKSTECHLAB) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_BUNKER) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_ENGINEERINGBAY) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_FACTORY) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_FACTORYFLYING) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_FACTORYREACTOR) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_FACTORYTECHLAB) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_FUSIONCORE) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_GHOSTACADEMY) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_MISSILETURRET) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_ORBITALCOMMAND) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_PLANETARYFORTRESS) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_REACTOR) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_REFINERY) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_SENSORTOWER) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_STARPORT) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_STARPORTFLYING) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_STARPORTREACTOR) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_STARPORTTECHLAB) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_SUPPLYDEPOT) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_SUPPLYDEPOTLOWERED) ||
			(unit->unit_type == UNIT_TYPEID::TERRAN_TECHLAB) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_DARKSHRINE) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_FLEETBEACON) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_FORGE) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_GATEWAY) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_PHOTONCANNON) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_PYLON) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_PYLONOVERCHARGED) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_ROBOTICSBAY) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_SHIELDBATTERY) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_STARGATE) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL) ||
			(unit->unit_type == UNIT_TYPEID::PROTOSS_WARPGATE) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_BANELINGNEST) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_CREEPTUMOR) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_CREEPTUMORBURROWED) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_EVOLUTIONCHAMBER) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_EXTRACTOR) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_GREATERSPIRE) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_HATCHERY) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_HIVE) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_HYDRALISKDEN) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_INFESTATIONPIT) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_LAIR) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_LURKERDENMP) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_NYDUSCANAL) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_NYDUSNETWORK) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_ROACHWARREN) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_SPAWNINGPOOL) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_SPINECRAWLER) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_SPINECRAWLERUPROOTED) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_SPIRE) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_SPORECRAWLER) ||
			(unit->unit_type == UNIT_TYPEID::ZERG_SPORECRAWLERUPROOTED))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UnitData::isFightingUnit(const Unit *unit)
{
	/* unimplemented
	if ()
	{
		return true;
	}
	else 
	{
		return false;
	}
	*/
	return false;
}

bool UnitData::isTownHall(const Unit *unit)
{

	if (unit->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTER || 
		unit->unit_type == UNIT_TYPEID::TERRAN_ORBITALCOMMAND ||
		unit->unit_type == UNIT_TYPEID::TERRAN_COMMANDCENTERFLYING ||
		unit->unit_type == UNIT_TYPEID::TERRAN_ORBITALCOMMANDFLYING ||
		unit->unit_type == UNIT_TYPEID::TERRAN_PLANETARYFORTRESS ||
		unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS|| 
		unit->unit_type == UNIT_TYPEID::ZERG_HATCHERY || 
		unit->unit_type == UNIT_TYPEID::ZERG_LAIR ||
		unit->unit_type == UNIT_TYPEID::ZERG_HIVE)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UnitData::isSupplyProvider(const Unit *unit)
{
	/* unimplemented
	if ()
	{
	return true;
	}
	else
	{
	return false;
	}
	*/
	return false;
}

bool UnitData::isMinerals(const Unit *unit)
{
	if ((unit->unit_type == UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD) ||
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_BATTLESTATIONMINERALFIELD750) ||
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_LABMINERALFIELD) ||
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_LABMINERALFIELD750) || 
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD) || 
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD750) || 
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD) || 
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERMINERALFIELD750) || 
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD) || 
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERRICHMINERALFIELD750) || 
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD) || 
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_RICHMINERALFIELD750))
	{
	return true;
	}
	else
	{
	return false;
	}
}

bool UnitData::isVespeneGeyser(const Unit *unit)
{
	if ((unit->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER) ||
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_PROTOSSVESPENEGEYSER) ||
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_PURIFIERVESPENEGEYSER) ||
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_SHAKURASVESPENEGEYSER) ||
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_RICHVESPENEGEYSER) ||
		(unit->unit_type == UNIT_TYPEID::NEUTRAL_SPACEPLATFORMGEYSER))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UnitData::isOurs(const Unit *unit)
{
	if (unit->alliance == Unit::Alliance::Self)
	{
		return true;
	}
	else 
	{
		return false;
	}
}

//returns true if the unit can cloak, also returns true for lurkers as they can attack while burrowed
 bool UnitData::canCloak(const Unit *unit)
{
	 if ((unit->unit_type == UNIT_TYPEID::TERRAN_BANSHEE) ||
		 (unit->unit_type == UNIT_TYPEID::TERRAN_GHOST) ||
		 (unit->unit_type == UNIT_TYPEID::PROTOSS_DARKTEMPLAR) ||
		 (unit->unit_type == UNIT_TYPEID::PROTOSS_MOTHERSHIP) ||
		 (unit->unit_type == UNIT_TYPEID::PROTOSS_OBSERVER) ||
		 (unit->unit_type == UNIT_TYPEID::ZERG_LURKERMP) ||
		 (unit->unit_type == UNIT_TYPEID::ZERG_LURKERMPBURROWED) ||
		 (unit->unit_type == UNIT_TYPEID::ZERG_LURKERMPEGG))
	 {
		 return true;
	 }
	 else
	 {
		 return false;
	 }
}

//returns which structure is required to produce a given ability(limited implementation)
UnitTypeID UnitData::requiredStructure(AbilityID ability)
{
	UnitTypeID required;

	switch (ABILITY_ID(ability))
	{
	case ABILITY_ID::RESEARCH_WARPGATE:
		required = UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
		break;
	case ABILITY_ID::RESEARCH_BLINK:
		required = UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL;
		break;
	case ABILITY_ID::RESEARCH_EXTENDEDTHERMALLANCE:
		required = UNIT_TYPEID::PROTOSS_ROBOTICSBAY;
		break;
	case ABILITY_ID::RESEARCH_PROTOSSGROUNDARMOR:
	case ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL1:
	case ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL2:
	case ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL3:
	case ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS:
	case ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL1:
	case ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL2:
	case ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL3:
		required = UNIT_TYPEID::PROTOSS_FORGE;
		break;
	case ABILITY_ID::TRAIN_ZEALOT:
		required = UNIT_TYPEID::PROTOSS_GATEWAY;
		break;
	case ABILITY_ID::TRAIN_STALKER:
		required = UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
		break;
	case ABILITY_ID::TRAIN_OBSERVER:
		required = UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY;
		break;
	case ABILITY_ID::TRAIN_COLOSSUS:
		required = UNIT_TYPEID::PROTOSS_ROBOTICSBAY;
		break;
	default:
		UNIT_TYPEID::INVALID;
	}

	return required;
}

//returns true if the ability is a research (limited implementation)
bool UnitData::isResearch(AbilityID ability)
{
	if ((ABILITY_ID(ability) == ABILITY_ID::RESEARCH_WARPGATE) ||
		(ABILITY_ID(ability) == ABILITY_ID::RESEARCH_BLINK) ||
		(ABILITY_ID(ability) == ABILITY_ID::RESEARCH_EXTENDEDTHERMALLANCE) ||
		(ABILITY_ID(ability) == ABILITY_ID::RESEARCH_PROTOSSGROUNDARMOR) ||
		(ABILITY_ID(ability) == ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL1) ||
		(ABILITY_ID(ability) == ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL2) ||
		(ABILITY_ID(ability) == ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL3) ||
		(ABILITY_ID(ability) == ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS) ||
		(ABILITY_ID(ability) == ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL1) ||
		(ABILITY_ID(ability) == ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL2) ||
		(ABILITY_ID(ability) == ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL3))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//checks if an AbilityID represents a trainable unit (only process implemented so far)
bool UnitData::isTrainableUnitType(AbilityID ability)
{
	if ((ABILITY_ID(ability) == ABILITY_ID::TRAINWARP_ADEPT) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAINWARP_DARKTEMPLAR) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAINWARP_HIGHTEMPLAR) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAINWARP_SENTRY) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAINWARP_STALKER) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAINWARP_ZEALOT) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_ADEPT) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_CARRIER) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_COLOSSUS) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_DARKTEMPLAR) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_DISRUPTOR) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_HIGHTEMPLAR) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_IMMORTAL) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_MOTHERSHIP) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_MOTHERSHIPCORE) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_OBSERVER) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_ORACLE) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_PHOENIX) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_PROBE) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_WARPPRISM) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_VOIDRAY) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_ZEALOT) ||
		(ABILITY_ID(ability) == ABILITY_ID::TRAIN_STALKER))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//returns a UnitTypeID based on a given AbilityID (only protoss implemented)
UnitTypeID UnitData::getUnitTypeID(AbilityID ability)
{
	switch (ABILITY_ID(ability))
	{
	case ABILITY_ID::TRAINWARP_ADEPT:
	case ABILITY_ID::TRAIN_ADEPT:
		return UNIT_TYPEID::PROTOSS_ADEPT;
		break;
	case ABILITY_ID::TRAINWARP_DARKTEMPLAR:
	case ABILITY_ID::TRAIN_DARKTEMPLAR:
		return UNIT_TYPEID::PROTOSS_DARKTEMPLAR;
		break;
	case ABILITY_ID::TRAINWARP_HIGHTEMPLAR:
	case ABILITY_ID::TRAIN_HIGHTEMPLAR:
		return UNIT_TYPEID::PROTOSS_HIGHTEMPLAR;
		break;
	case ABILITY_ID::TRAINWARP_SENTRY:
		return UNIT_TYPEID::PROTOSS_SENTRY;
		break;
	case ABILITY_ID::TRAINWARP_STALKER:
	case ABILITY_ID::TRAIN_STALKER:
		return UNIT_TYPEID::PROTOSS_STALKER;
		break;
	case ABILITY_ID::TRAINWARP_ZEALOT:
	case ABILITY_ID::TRAIN_ZEALOT:
		return UNIT_TYPEID::PROTOSS_ZEALOT;
		break;
	case ABILITY_ID::TRAIN_CARRIER:
		return UNIT_TYPEID::PROTOSS_CARRIER;
		break;
	case ABILITY_ID::TRAIN_COLOSSUS:
		return UNIT_TYPEID::PROTOSS_COLOSSUS;
		break;
	case ABILITY_ID::TRAIN_DISRUPTOR:
		return UNIT_TYPEID::PROTOSS_DISRUPTOR;
		break;
	case ABILITY_ID::TRAIN_IMMORTAL:
		return UNIT_TYPEID::PROTOSS_IMMORTAL;
		break;
	case ABILITY_ID::TRAIN_MOTHERSHIP:
		return UNIT_TYPEID::PROTOSS_MOTHERSHIP;
		break;
	case ABILITY_ID::TRAIN_MOTHERSHIPCORE:
		return UNIT_TYPEID::PROTOSS_MOTHERSHIPCORE;
		break;
	case ABILITY_ID::TRAIN_OBSERVER:
		return UNIT_TYPEID::PROTOSS_OBSERVER;
		break;
	case ABILITY_ID::TRAIN_ORACLE:
		return UNIT_TYPEID::PROTOSS_ORACLE;
		break;
	case ABILITY_ID::TRAIN_PHOENIX:
		return UNIT_TYPEID::PROTOSS_PHOENIX;
		break;
	case ABILITY_ID::TRAIN_PROBE:
		return UNIT_TYPEID::PROTOSS_PROBE;
		break;
	case ABILITY_ID::TRAIN_WARPPRISM:
		return UNIT_TYPEID::PROTOSS_WARPPRISM;
		break;
	case ABILITY_ID::TRAIN_VOIDRAY:
		return UNIT_TYPEID::PROTOSS_VOIDRAY;
		break;
	case ABILITY_ID::BUILD_ASSIMILATOR:
		return UNIT_TYPEID::PROTOSS_ASSIMILATOR;
		break;
	case ABILITY_ID::BUILD_CYBERNETICSCORE:
		return UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
		break;
	case ABILITY_ID::BUILD_DARKSHRINE:
		return UNIT_TYPEID::PROTOSS_DARKSHRINE;
		break;
	case ABILITY_ID::BUILD_FLEETBEACON:
		return UNIT_TYPEID::PROTOSS_FLEETBEACON;
		break;
	case ABILITY_ID::BUILD_FORGE:
		return UNIT_TYPEID::PROTOSS_FORGE;
		break;
	case ABILITY_ID::BUILD_NEXUS:
		return UNIT_TYPEID::PROTOSS_NEXUS;
		break;
	case ABILITY_ID::BUILD_PHOTONCANNON:
		return UNIT_TYPEID::PROTOSS_PHOTONCANNON;
		break;
	case ABILITY_ID::BUILD_PYLON:
		return UNIT_TYPEID::PROTOSS_PYLON;
		break;
	case ABILITY_ID::BUILD_ROBOTICSBAY:
		return UNIT_TYPEID::PROTOSS_ROBOTICSBAY;
		break;
	case ABILITY_ID::BUILD_ROBOTICSFACILITY:
		return UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY;
		break;
	case ABILITY_ID::BUILD_SHIELDBATTERY:
		return UNIT_TYPEID::PROTOSS_SHIELDBATTERY;
		break;
	case ABILITY_ID::BUILD_STARGATE:
		return UNIT_TYPEID::PROTOSS_STARGATE;
		break;
	case ABILITY_ID::BUILD_TEMPLARARCHIVE:
		return UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE;
		break;
	case ABILITY_ID::BUILD_TWILIGHTCOUNCIL:
		return UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL;
		break;
	default:
		return UNIT_TYPEID::INVALID;
		break;
	}
}

//returns true if the ability refers to a structure that a worker can build
bool UnitData::isBuildAbility(AbilityID ability)
{
	if (ability == ABILITY_ID::BUILD_ARMORY ||
		ability == ABILITY_ID::BUILD_ASSIMILATOR ||
		ability == ABILITY_ID::BUILD_BANELINGNEST ||
		ability == ABILITY_ID::BUILD_BARRACKS ||
		ability == ABILITY_ID::BUILD_BUNKER ||
		ability == ABILITY_ID::BUILD_COMMANDCENTER ||
		ability == ABILITY_ID::BUILD_CYBERNETICSCORE ||
		ability == ABILITY_ID::BUILD_DARKSHRINE ||
		ability == ABILITY_ID::BUILD_ENGINEERINGBAY ||
		ability == ABILITY_ID::BUILD_EVOLUTIONCHAMBER ||
		ability == ABILITY_ID::BUILD_EXTRACTOR ||
		ability == ABILITY_ID::BUILD_FACTORY ||
		ability == ABILITY_ID::BUILD_FLEETBEACON ||
		ability == ABILITY_ID::BUILD_FORGE ||
		ability == ABILITY_ID::BUILD_FUSIONCORE ||
		ability == ABILITY_ID::BUILD_GATEWAY ||
		ability == ABILITY_ID::BUILD_GHOSTACADEMY ||
		ability == ABILITY_ID::BUILD_HATCHERY ||
		ability == ABILITY_ID::BUILD_HYDRALISKDEN ||
		ability == ABILITY_ID::BUILD_INFESTATIONPIT ||
		ability == ABILITY_ID::BUILD_MISSILETURRET ||
		ability == ABILITY_ID::BUILD_NEXUS ||
		ability == ABILITY_ID::BUILD_NYDUSNETWORK ||
		ability == ABILITY_ID::BUILD_PHOTONCANNON ||
		ability == ABILITY_ID::BUILD_PYLON ||
		ability == ABILITY_ID::BUILD_REFINERY ||
		ability == ABILITY_ID::BUILD_ROACHWARREN ||
		ability == ABILITY_ID::BUILD_ROBOTICSBAY ||
		ability == ABILITY_ID::BUILD_ROBOTICSFACILITY ||
		ability == ABILITY_ID::BUILD_SENSORTOWER ||
		ability == ABILITY_ID::BUILD_SHIELDBATTERY ||
		ability == ABILITY_ID::BUILD_SPAWNINGPOOL ||
		ability == ABILITY_ID::BUILD_SPINECRAWLER ||
		ability == ABILITY_ID::BUILD_SPIRE ||
		ability == ABILITY_ID::BUILD_SPORECRAWLER ||
		ability == ABILITY_ID::BUILD_STARGATE ||
		ability == ABILITY_ID::BUILD_STARPORT ||
		ability == ABILITY_ID::BUILD_SUPPLYDEPOT ||
		ability == ABILITY_ID::BUILD_TEMPLARARCHIVE ||
		ability == ABILITY_ID::BUILD_TWILIGHTCOUNCIL ||
		ability == ABILITY_ID::BUILD_ULTRALISKCAVERN)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//returns true for Assimilators, Extractors, and Refineries
bool UnitData::isGasStructure(const Unit *unit)
{
	if (unit->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR ||
		unit->unit_type == UNIT_TYPEID::TERRAN_REFINERY ||
		unit->unit_type == UNIT_TYPEID::ZERG_EXTRACTOR)
	{
		return true;
	}
	else
	{
		return false;
	}
}