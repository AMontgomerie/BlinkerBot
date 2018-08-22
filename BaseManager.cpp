#include "BaseManager.h"
#include "Blinkerbot.h"


BaseManager::BaseManager(BlinkerBot & bot): blinkerBot(bot)
{
}


BaseManager::~BaseManager()
{
}

Base::Base()
{
}
Base::~Base()
{
}

Base::Base(std::set<const Unit *> minerals, std::set<const Unit *> geysers, Point2D buildLocation)
{
	this->minerals = minerals;
	this->geysers = geysers;
	this->buildLocation = buildLocation;
	this->townhall = nullptr;

}

Base::Base(std::set<const Unit *> minerals, std::set<const Unit *> geysers, Point2D buildLocation, const Unit *townhall)
{
	this->minerals = minerals;
	this->geysers = geysers;
	this->buildLocation = buildLocation;
	this->townhall = townhall;

}

void BaseManager::initialise()
{
	findBases();
	printDebug();
}

//searches the map for minerals and groups them into mineral lines, the results are stored in mineralLines
std::vector<std::vector<const Unit *>> BaseManager::findMineralLines()
{
	std::vector<std::vector<const Unit *>> mineralLines;

	//check everything on the map
	for (auto unit : blinkerBot.Observation()->GetUnits())
	{
		//if we find a mineral patch
		if (UnitData::isMinerals(unit))
		{
			bool groupFound = false;

			//iterate through each mineral line we know about
			std::vector<std::vector<const Unit *>>::iterator line = mineralLines.begin();
			while (!groupFound && line != mineralLines.end())
			{
				//check the distant from our new mineral patch to each member we know about in the line
				std::vector<const Unit *>::iterator mineral = (*line).begin();
				while (!groupFound && mineral != (*line).end())
				{
					if (Distance2D((*mineral)->pos, unit->pos) < 12)
					{
						//if the new mineral patch is part of a previously known mineral line, at it to the vector
						(*line).push_back(unit);
						groupFound = true;
					}
					else
					{
						mineral++;
					}
				}
				line++;
			}
			//if it wasn't part of a mineral line we know about, make a new mineral line
			if (!groupFound)
			{
				std::vector<const Unit *> newLine;
				newLine.push_back(unit);
				mineralLines.push_back(newLine);
			}
		}
	}

	return mineralLines;
}

void BaseManager::printDebug()
{
	/*
	std::cerr << "DISTANCE TO EACH SET OF MINERALS" << std::endl;
	for (auto base : bases)
	{
		std::cerr << Distance2D(blinkerBot.Observation()->GetStartLocation(), (*base.getMinerals().begin())->pos)
			<< " from " << blinkerBot.Observation()->GetStartLocation().x << ":" << blinkerBot.Observation()->GetStartLocation().y
			<< " to " << (*base.getMinerals().begin())->pos.x << ":" << (*base.getMinerals().begin())->pos.y << std::endl;
	}

	std::cerr << "" << std::endl;

	std::cerr << "DISTANCE TO EACH BASE LOCATION" << std::endl;

	for (auto base : bases)
	{
		std::cerr << Distance2D(blinkerBot.Observation()->GetStartLocation(), base.getBuildLocation())
			<< " from " << blinkerBot.Observation()->GetStartLocation().x << ":" << blinkerBot.Observation()->GetStartLocation().y
			<< " to " << base.getBuildLocation().x << ":" << base.getBuildLocation().y << std::endl;
	}
	*/
	/*
	for (int i = 0; i != ourBases.size(); i++)
	{
		std::ostringstream ourBase;
		ourBase << "base " << i + 1 << ": at " << ourBases[i].getBuildLocation().x << ":" << ourBases[i].getBuildLocation().y << std::endl;
		blinkerBot.Debug()->DebugTextOut(ourBase.str());
	}
	*/
	/*
	int n = 0;
	for (auto base : bases)
	{
		blinkerBot.Debug()->DebugBoxOut(Point3D(base.getBuildLocation().x - 3, base.getBuildLocation().y - 3, blinkerBot.Observation()->GetStartLocation().z),
			Point3D(base.getBuildLocation().x + 3, base.getBuildLocation().y + 3, blinkerBot.Observation()->GetStartLocation().z - 3), getDebugColor(n));
		for (auto mineral : base.getMinerals())
		{
			blinkerBot.Debug()->DebugBoxOut(Point3D(mineral->pos.x, mineral->pos.y, blinkerBot.Observation()->GetStartLocation().z + 1),
				Point3D(mineral->pos.x + 1, mineral->pos.y + 1, blinkerBot.Observation()->GetStartLocation().z - 3), getDebugColor(n));
		}
		for (auto gas : base.getGeysers())
		{
			blinkerBot.Debug()->DebugBoxOut(Point3D(gas->pos.x, gas->pos.y, blinkerBot.Observation()->GetStartLocation().z + 1),
				Point3D(gas->pos.x + 2, gas->pos.y + 2, blinkerBot.Observation()->GetStartLocation().z - 3), getDebugColor(n));
		}
		n++;
	}*/

	//blinkerBot.Debug()->DebugShowMap();
	//blinkerBot.Debug()->SendDebug();
}

std::vector<Point2D> BaseManager::getBuildGrid(Point2D centre)
{
	std::vector<QueryInterface::PlacementQuery> queries;
	std::vector<Point2D> buildGrid;

	//first we set the minimum and maximum values for the search area
	float minX = centre.x - 10;
	float minY = centre.y - 10;
	float maxX = centre.x + 10;
	float maxY = centre.y + 10;

	if (minX < 0)
	{
		minX = 0;
	}
	if (minY < 0)
	{
		minY = 0;
	}
	if (maxX > blinkerBot.Observation()->GetGameInfo().width)
	{
		maxX = float(blinkerBot.Observation()->GetGameInfo().width);
	}
	if (maxY > blinkerBot.Observation()->GetGameInfo().width)
	{
		maxY = float(blinkerBot.Observation()->GetGameInfo().width);
	}

	//load up the vector with a query for each point
	for (float x = minX; x <= maxX; x++)
	{
		for (float y = minY; y <= maxY; y++)
		{
			queries.push_back(QueryInterface::PlacementQuery(ABILITY_ID::BUILD_NEXUS, Point2D(x, y)));
			//if (blinkerBot.Observation()->IsPlacable(Point2D(x, y)))
			//{
			buildGrid.push_back(Point2D(x, y));
			//}
		}
	}

	//query each position
	std::vector<bool> output = blinkerBot.Query()->Placement(queries);

	if (!output.empty())
	{
		//if a point is unbuildable, set its location to 0:0 so we know it doesn't work
		for (int i = 0; i != output.size() - 1; ++i)
		{
			//std::cerr << i << ": " << output[i] << std::endl;
			if (output[i] == false)
			{
				buildGrid[i] = Point2D(0, 0);
			}
		}
	}
	//go through the vector and remove all the unbuildable (0:0) points
	for (std::vector<Point2D>::iterator point = buildGrid.begin(); point != buildGrid.end();)
	{
		if (*point == Point2D(0, 0))
		{
			point = buildGrid.erase(point);
		}
		else
		{
			++point;
		}
	}

	for (std::vector<Point2D>::iterator point = buildGrid.begin(); point != buildGrid.end();)
	{
		bool tooClose = false;
		//let's make sure it's not too close to our resources (we don't wanna block any mineral or gas lines
		for (auto resource : blinkerBot.Observation()->GetUnits())
		{
			if ((UnitData::isMinerals(resource) || UnitData::isVespeneGeyser(resource)) && Distance2D(*point, resource->pos) < 4)
			{
				//std::cerr << "removing a location for being too close to a mineral/gas" << std::endl;
				tooClose = true;
			}
		}
		if (tooClose)
		{
			point = buildGrid.erase(point);
		}
		else
		{
			++point;
		}
	}

	//printBuildGrid(buildGrid);
	return buildGrid;
}

//prints out a set of squares representing buildable points
void BaseManager::printBuildGrid(std::vector<Point2D> buildGrid)
{
	for (auto point : buildGrid)
	{
		Point3D box = Point3D(point.x, point.y, blinkerBot.Observation()->GetStartLocation().z + 1);
		blinkerBot.Debug()->DebugBoxOut(box, Point3D(box.x + 1, box.y + 1, box.z), Colors::Purple);
	}
	blinkerBot.Debug()->SendDebug();
}

Color BaseManager::getDebugColor(int num)
{
	if (num < 0)
	{
		return Colors::Black;
	}
	else if (num > 9)
	{
		num = num % 10;
	}
	switch (num)
	{
	case 0:
		return Colors::Black;
		break;
	case 1:
		return Colors::Blue;
		break;
	case 2:
		return Colors::Gray;
		break;
	case 3:
		return Colors::Green;
		break;
	case 4:
		return Colors::Purple;
		break;
	case 5:
		return Colors::Red;
		break;
	case 6:
		return Colors::Teal;
		break;
	case 7:
		return Colors::White;
		break;
	case 8:
		return Colors::Yellow;
		break;
	case 9:
		return Colors::Purple;
		break;
	default:
		return Colors::Black;
	}
}

//returns the set of points in ascending order of distance from our main
std::vector<Base> BaseManager::sortBaseLocations(std::vector<Base> baseLocations)
{
	std::vector<Base> sorted;
	size_t size = baseLocations.size();
	//std::cerr << size << " base locations found" << std::endl;
	//for each base location
	for (int i = 0; i != size; i++)
	{
		//find the closest base to our start location
		Base closestBase = *baseLocations.begin();
		int closestIndex = 0;
		int count = 0;
		for (auto base : baseLocations)
		{
			//get a worker to be our distance tester
			const Unit *worker = *blinkerBot.Observation()->GetUnits().begin();
			for (auto unit : blinkerBot.Observation()->GetUnits())
			{
				if (UnitData::isOurs(unit) && UnitData::isWorker(unit))
				{
					worker = unit;
				}
			}
			//std::cerr << "query: " << blinkerBot.Query()->PathingDistance(worker, base.getBuildLocation()) << std::endl;
			//std::cerr << "dist: " << Distance2D(blinkerBot.Observation()->GetStartLocation(), base.getBuildLocation()) << std::endl;

			if (blinkerBot.Query()->PathingDistance(worker, base.getBuildLocation()) < 
				blinkerBot.Query()->PathingDistance(worker, closestBase.getBuildLocation()))				
			{
				closestBase = base;
				closestIndex = count;
			}
			count++;
		}
		//remove the closest base from the vector
		baseLocations.erase(baseLocations.begin() + closestIndex);

		//add our closest base to the new vector, then loop and find the next closest
		sorted.push_back(closestBase);		
	}

	/*
	const Unit *worker = *blinkerBot.Observation()->GetUnits().begin();
	for (auto unit : blinkerBot.Observation()->GetUnits())
	{
		if (UnitData::isOurs(unit) && UnitData::isWorker(unit))
		{
			worker = unit;
		}
	}

	for (auto base : sorted)
	{
		std::cerr << blinkerBot.Query()->PathingDistance(worker, calculateGasAverage(base.getGeysers())) << std::endl;
	}
	*/
	
	return sorted;

}

std::set<const Unit *> BaseManager::findGeysers()
{
	std::set<const Unit *> geysers;
	for (auto unit : blinkerBot.Observation()->GetUnits())
	{
		if (UnitData::isVespeneGeyser(unit))
		{
			geysers.insert(unit);
		}
	}
	//std::cerr << geysers.size() << " geysers found" << std::endl;
	return geysers;
}

const Unit *BaseManager::getNextAvailableGas()
{
	for (auto base : ourBases)
	{
		for (auto geyser : base.getGeysers())
		{
			bool alreadyTaken = false;
			for (auto ourGas : ourGases)
			{
				if ((geyser->pos.x == ourGas->pos.x) && (geyser->pos.y == ourGas->pos.y))
				{
					alreadyTaken = true;
				}
			}
			if (!alreadyTaken)
			{
				return geyser;
			}
		}
	}
	std::cerr << "No available geysers" << std::endl;
	return nullptr;
}

std::vector<Base> BaseManager::getAvailableBaseLocations()
{
	return availableBases;
}

Point2D BaseManager::getNextBaseLocation()
{
	/*
	Base nextBase = *availableBases.begin();

	//if we haven't calculated the base location yet, we need to do it before we pass it to production manager
	//if (nextBase.getBuildLocation() == (*nextBase.getMinerals().begin())->pos)
	if (nextBase.getBuildLocation() == calculateGasAverage(nextBase.getGeysers()))
	{
		//make a new base with the same minerals and gas and an accurate location
		Base accurateBase = Base(nextBase.getMinerals(), nextBase.getGeysers(), calculateBuildLocation(nextBase));
		//throw away the old version
		removeNextBaseLocation();
		//put the new base at the front
		availableBases.insert(availableBases.begin(), accurateBase);
	}
	return (*availableBases.begin()).getBuildLocation();
	*/

	if (availableBases.empty())
	{
		std::cerr << "BaseManager::getNextBaseLocation(): No available expansions." << std::endl;
		return blinkerBot.Observation()->GetStartLocation();
	}

	std::vector<Base>::iterator nextBase = availableBases.begin();

	//find a worker to test with
	const Unit *worker = nullptr;
	for (auto unit : blinkerBot.Observation()->GetUnits())
	{
		if (UnitData::isOurs(unit) && UnitData::isWorker(unit))
		{
			worker = unit;
		}
	}
	if (worker)
	{
		//check if our worker can get to the closest base
		bool pathable = false;
		while (!pathable && nextBase != availableBases.end())
		{
			if (blinkerBot.Query()->PathingDistance(worker, (*nextBase).getBuildLocation()) == 0)
			{
				//std::cerr << "unreachable base at " << (*nextBase).getBuildLocation().x << ":" << (*nextBase).getBuildLocation().y << std::endl;
				nextBase = availableBases.erase(nextBase);
			}
			else
			{
				pathable = true;
			}
		}
		//if we have no reachable bases left, exit
		if (availableBases.empty())
		{
			std::cerr << "BaseManager::getNextBaseLocation(): No available expansions." << std::endl;
			return blinkerBot.Observation()->GetStartLocation();
		}
	}

	//if we haven't calculated the base location yet, we need to do it before we pass it to production manager
	//if (nextBase.getBuildLocation() == (*nextBase.getMinerals().begin())->pos)
	if ((*nextBase).getBuildLocation() == calculateAveragePoint((*nextBase).getGeysers()))
	{
		//make a new base with the same minerals and gas and an accurate location
		Base accurateBase = Base((*nextBase).getMinerals(), (*nextBase).getGeysers(), calculateBuildLocation(*nextBase));
		//throw away the old version
		removeNextBaseLocation();
		//put the new base at the front
		availableBases.insert(availableBases.begin(), accurateBase);
	}
	return (*availableBases.begin()).getBuildLocation();
}

/*
takes a set of minerals and gas and returns an acceptable expansion location
*/
Point2D BaseManager::calculateBuildLocation(Base base)
{
	//first we get a vector of nearby buildable locations (EXPENSIVE)
	std::vector<Point2D> buildGrid = getBuildGrid(base.getBuildLocation());

	//search through the locations we receive and try to find the closest point that isn't too close
	Point2D closest = *buildGrid.begin();
	for (auto point : buildGrid)
	{
		if (Distance2D(point, (*base.getMinerals().begin())->pos) < Distance2D(closest, (*base.getMinerals().begin())->pos))
		{
			bool invalid = false;
			for (auto mineral : base.getMinerals())
			{
				if (Distance2D(mineral->pos, point) < 3 || Distance2D(mineral->pos, point) > 10)
				{
					invalid = true;
				}
			}
			for (auto geyser : base.getGeysers())
			{
				if (Distance2D(geyser->pos, point) < 3 || Distance2D(geyser->pos, point) > 10)
				{
					invalid = true;
				}
			}
			if (!invalid)
			{
				closest = point;
			}
		}
	}
	printDebug();
	return closest;
}

//called when we start a new base so we can remove it from the group of available locations
void BaseManager::removeNextBaseLocation()
{
	availableBases.erase(availableBases.begin());
}

//called when a base is destroyed to make it available again so we can rebuild
void BaseManager::addAvailableBaseLocation(const Unit * unit)
{
	for (auto base : bases)
	{
		if (base.getTownhall() == unit)
		{
			availableBases.push_back(base);
		}
	}
	availableBases = sortBaseLocations(availableBases);
}

std::set<const Unit *> Base::getMinerals()
{
	return minerals;
}

std::set<const Unit *> Base::getGeysers()
{
	return geysers;
}

Point2D Base::getBuildLocation()
{
	return buildLocation;
}

const Unit *Base::getTownhall()
{
	return townhall;
}

/*
takes a set and returns an average point given a set of geysers or minerals. Used for initial base location estimations
*/
Point2D BaseManager::calculateAveragePoint(std::set<const Unit *> nodes)
{
	float averageX = 0;
	float averageY = 0;
	for (auto node : nodes)
	{
		averageX += node->pos.x;
		averageY += node->pos.y;
	}
	averageX = averageX / nodes.size();
	averageY = averageY / nodes.size();
	return Point2D(averageX, averageY);
}

/*
takes a vector and returns an average point given a set of geysers or minerals. Used for initial base location estimations
*/
Point2D BaseManager::calculateAveragePoint(std::vector<const Unit *> nodes)
{
	float averageX = 0;
	float averageY = 0;
	for (auto node : nodes)
	{
		averageX += node->pos.x;
		averageY += node->pos.y;
	}
	averageX = averageX / nodes.size();
	averageY = averageY / nodes.size();
	return Point2D(averageX, averageY);
}

/*
find all of the Bases on the map and updates the bases vector with them
*/
void BaseManager::findBases()
{
	std::vector<std::vector<const Unit *>> allMineralLines = findMineralLines();
	std::set<const Unit *> allGeysers = findGeysers();

	for (auto line : allMineralLines)
	{
		Point2D baseLocation;
		std::set<const Unit *> mineralLine;
		std::set<const Unit *> geysers;

		//find the geysers
		/*
		for (auto geyser : allGeysers)
		{
			if (Distance2D(geyser->pos, (*line.begin())->pos) < 12)
			{
				geysers.insert(geyser);
			}
		}*/

		geysers = findTwoClosestGeysers(allGeysers, calculateAveragePoint(line));

		//find the minerals
		for (auto mineral : line)
		{
			mineralLine.insert(mineral);
		}

		//sort the minerals by distance from an average point
		std::vector<const Unit *> sortedLine = sortMineralLine(mineralLine, calculateAveragePoint(geysers));
		mineralLine.clear();

		//feed in the 8 closest mineral nodes and ignore any extra ones as they're mistakenly grouped from another nearby base
		for (int i = 0; i < sortedLine.size() && i < 8; i++)
		{
			mineralLine.insert(sortedLine[i]);
		}

		//find the build location
		baseLocation = calculateAveragePoint(geysers);
		//baseLocation = (*mineralLine.begin())->pos;

		//the placement will be messed up for our main base since there's already a nexus there, so let's just add the start location instead
		if (Distance2D((*mineralLine.begin())->pos, blinkerBot.Observation()->GetStartLocation()) < 15)
		{
			bases.push_back(Base(mineralLine, geysers, blinkerBot.Observation()->GetStartLocation()));
		}
		else
		{
			bases.push_back(Base(mineralLine, geysers, baseLocation));
		}
	}

	/*now we've found all the bases, let's use them to make a vector sorted in order of distance from our main so that
	we know where to expand to*/
	std::vector<Base> unsortedBases;
	for (auto base : bases)
	{
		//when we find our main base, set the unit pointer rather than adding it to the available bases
		if (Distance2D(base.getBuildLocation(), blinkerBot.Observation()->GetStartLocation()) < 20)
		{
			for (auto unit : blinkerBot.Observation()->GetUnits())
			{
				if (unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS)
				{
					base.setTownhall(unit);
				}
			}
		}
		//otherwise add it to the group of available bases
		else
		{
			unsortedBases.push_back(base);
		}
	}

	//now we have all the available bases, let's sort them in order of distance from our main
	availableBases = sortBaseLocations(unsortedBases);

}

/*
finds the closest two geysers to the given base location and returns them as a set
*/
std::set<const Unit *> BaseManager::findTwoClosestGeysers(std::set<const Unit *> geysers, Point2D baseLocation)
{
	const Unit *closestGeyser = *geysers.begin();
	const Unit *nextClosestGeyser = *geysers.begin();
	std::set<const Unit *> closestPair;

	//find the closest geyser to the target
	for (auto geyser : geysers)
	{
		if (Distance2D(geyser->pos, baseLocation) < Distance2D(closestGeyser->pos, baseLocation))
		{
			closestGeyser = geyser;
		}
	}
	//find the closest geyser to the target that isn't the first one we found
	for (auto geyser : geysers)
	{
		if (geyser != closestGeyser && Distance2D(geyser->pos, baseLocation) < Distance2D(nextClosestGeyser->pos, baseLocation))
		{
			nextClosestGeyser = geyser;
		}
	}
	closestPair.insert(closestGeyser);
	closestPair.insert(nextClosestGeyser);
	return closestPair;
}

/*
takes a set of pointers to minerals and a base location point, and returns a vector of the minerals sorted by their distance from the point
*/
std::vector<const Unit *> BaseManager::sortMineralLine(std::set<const Unit *> line, Point2D baseLocation)
{
	std::vector<const Unit *> sortedLine;
	size_t size = line.size();

	for (int i = 0; i != size; i++)
	{
		//find the closest mineral in the line
		const Unit *closestMineral = nullptr;
		for (auto mineral : line)
		{
			if (!closestMineral || Distance2D(mineral->pos, baseLocation) < Distance2D(closestMineral->pos, baseLocation))
			{
				closestMineral = mineral;
			}
		}
		//erase the closest mineral from the line
		for (std::set<const Unit *>::iterator mineral = line.begin(); mineral != line.end();)
		{
			if (*mineral == closestMineral)
			{
				line.erase(mineral++);
			}
			else
			{
				++mineral;
			}
		}
		//add the closest mineral to the sorted set
		sortedLine.push_back(closestMineral);
	}

	return sortedLine;
}

void Base::setTownhall(const Unit *unit)
{
	this->townhall = unit;
}

void BaseManager::addBase(const Unit *unit)
{
	for (auto base : bases)
	{
		if (Distance2D((*base.getMinerals().begin())->pos, unit->pos) < 15)
		{
			bool alreadyFound = false;
			for (auto ourBase : ourBases)
			{
				if (Distance2D(base.getBuildLocation(), ourBase.getBuildLocation()) < 15)
				{
					alreadyFound = true;
				}
			}
			if (!alreadyFound)
			{
				base.setTownhall(unit);
				ourBases.push_back(base);
			}
		}
	}
	printDebug();
}

void BaseManager::removeBase(const Unit *unit)
{
	for (auto base : bases)
	{
		if (base.getTownhall() == unit)
		{
			base.setTownhall(nullptr);
		}
	}

	for (std::vector<Base>::iterator base = ourBases.begin(); base != ourBases.end();)
	{
		if ((*base).getTownhall() == unit)
		{
			base = ourBases.erase(base);
		}
		else
		{
			++base;
		}
	}

}

void BaseManager::addGas(const Unit *unit)
{
	ourGases.insert(unit);
}

void BaseManager::removeGas(const Unit *unit)
{
	for (std::set<const Unit *>::iterator gas = ourGases.begin(); gas != ourGases.end();)
	{
		if (*gas == unit)
		{
			ourGases.erase(gas++);
		}
		else
		{
			++gas;
		}
	}
}

std::vector<Base> BaseManager::getOurBases()
{
	return ourBases;
}

//we need to create a new base location when it's completed because initially stored base locations only store resources as snapshots
void BaseManager::updateCompletedBase(const Unit *unit)
{

	//remove the base that was previously in the vector so we can replace it with a new one
	removeBase(unit);

	std::set<const Unit *> minerals;
	std::set<const Unit *> geysers;

	//search for the base's minerals
	for (auto mineral : blinkerBot.Observation()->GetUnits())
	{
		//if we find a mineral patch
		if (UnitData::isMinerals(mineral) && Distance2D(mineral->pos, unit->pos) < 15)
		{
			minerals.insert(mineral);
		}
	}
	//search for the base's gas
	for (auto gas : blinkerBot.Observation()->GetUnits())
	{
		//if we find a mineral patch
		if (UnitData::isVespeneGeyser(gas) && Distance2D(gas->pos, unit->pos) < 15)
		{
			geysers.insert(gas);
		}
	}

	/*
	
	std::cerr << "the new base has: " << std::endl;

	int snapshot = 0;
	int visible = 0;
	for (auto mineral : minerals)
	{
		if (mineral->display_type == Unit::DisplayType::Snapshot)
		{
			snapshot++;
		}
		else
		{
			visible++;
		}
	}
	std::cerr << snapshot << " snapshot minerals and " << visible << " visible minerals" << std::endl;

	snapshot = 0;
	visible = 0;
	for (auto geyser : geysers)
	{
		if (geyser->display_type == Unit::DisplayType::Snapshot)
		{
			snapshot++;
		}
		else
		{
			visible++;
		}
	}
	std::cerr << snapshot << " snapshot geysers and " << visible << " visible geysers" << std::endl;
	*/

	//std::cerr << "adding a completed base to vector" << std::endl;
	ourBases.push_back(Base(minerals, geysers, unit->pos, unit));
}