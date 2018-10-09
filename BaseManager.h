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

	Point2D mainRampTop;
	Point2D mainRampBottom;
	Point2D wallInOpening;
	Point2D mainFirstWallinPosition;
	Point2D mainSecondWallinPosition;
	Point2D mainFirstPylonPosition;
	Point2D naturalFirstPylonPosition;
	std::vector<Point2D> wallInPoints;
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
	std::vector<Point2D> getBuildGrid(Point2D centre, int radius, AbilityID structure);
	Point2D calculateBuildLocation(Base base);
	Point2D calculateAveragePoint(std::set<const Unit *> nodes);
	Point2D calculateAveragePoint(std::vector<const Unit *> nodes);
	std::vector<const Unit *> sortMineralLine(std::set<const Unit *> line, Point2D baseLocation);
	std::set<const Unit *> findTwoClosestGeysers(std::set<const Unit *> geysers, Point2D baseLocation);
	void findRamp();
	void calculatePlacableRampTop(Point2D ramp);
	std::vector<Point2D> calculateGrid(Point2D centre, int size);
	bool isNextToRamp(Point2D point);
	//void calculateWallInOpening();
	void calculateMainWallInPositions();
	void calculateMainFirstPylonPosition();
	void calculateNaturalFirstPylonPosition();
	void calculateNaturalWallInPositions();
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
	Point2D getMainRampTop();
	Point2D getMainFirstPylonPosition();
	Point2D getMainFirstWallInPosition();
	Point2D getMainSecondWallInPosition();
	Point2D getNaturalFirstPylonPosition();
	std::vector<Point2D> getAllBaseLocations();
	bool mainFirstWallInPositionExists();
	bool mainSecondWallInPositionExists();
};

