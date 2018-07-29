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