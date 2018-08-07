#pragma once

#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"
#include "sc2lib/sc2_lib.h"
#include "UnitData.h"

using namespace sc2;

class BlinkerBot;

class Base
{
	std::set<const Unit *> minerals;
	std::set<const Unit *> geysers;
	Point2D buildLocation;
	const Unit *townhall;

public:
	Base(std::set<const Unit *> minerals, std::set<const Unit *> gas, Point2D buildLocation);
	Base(std::set<const Unit *> minerals, std::set<const Unit *> geysers, Point2D buildLocation, const Unit *townhall);
	Base();
	~Base();
	std::set<const Unit *> getMinerals();
	std::set<const Unit *> getGeysers();
	Point2D getBuildLocation();
	const Unit *getTownhall();
	void setTownhall(const Unit *unit);
};

class BaseManager
{
	BlinkerBot & blinkerBot;

	std::vector<Base> bases;
	std::vector<Base> availableBases;
	std::vector<Base> ourBases;
	std::set<const Unit *> ourGases;

	void findBases();
	std::vector<std::vector<const Unit *>> findMineralLines();
	std::set<const Unit *> findGeysers();
	std::vector<Base> sortBaseLocations(std::vector<Base> baseLocations);
	void printDebug();
	Color getDebugColor(int num);
	void printBuildGrid(std::vector<Point2D> buildGrid);
	std::vector<Point2D> getBuildGrid(Point2D centre);
	//std::vector<Point2D> getPreliminaryBuildGrid(Point2D centre);
	Point2D calculateBuildLocation(Base base);
public:
	BaseManager(BlinkerBot & bot);
	~BaseManager();
	void initialise();
	const Unit *getNextAvailableGas();
	std::vector<Base> getAvailableBaseLocations();
	Point2D getNextBaseLocation();
	void removeNextBaseLocation();
	void addAvailableBaseLocation(const Unit * unit);
	void addBase(const Unit *unit);
	void updateCompletedBase(const Unit *unit);
	void removeBase(const Unit *unit);
	void addGas(const Unit *unit);
	void removeGas(const Unit *unit);
	std::vector<Base> getOurBases();
};

