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
					if (Distance2D((*mineral)->pos, unit->pos) < 15)
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
	for (int i = 0; i != ourBases.size(); i++)
	{
		std::ostringstream ourBase;
		ourBase << "base " << i + 1 << ": at " << ourBases[i].getBuildLocation().x << ":" << ourBases[i].getBuildLocation().y << std::endl;
		blinkerBot.Debug()->DebugTextOut(ourBase.str());
	}


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
	}

	//blinkerBot.Debug()->DebugShowMap();
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
	std::cerr << size << " base locations found" << std::endl;
	//for each base location
	for (int i = 0; i != size; i++)
	{
		//find the closest base to our start location
		Base closestBase = *baseLocations.begin();
		int closestIndex = 0;
		int count = 0;
		for (auto base : baseLocations)
		{
			if (Distance2D(blinkerBot.Observation()->GetStartLocation(), base.getBuildLocation()) <
				Distance2D(blinkerBot.Observation()->GetStartLocation(), closestBase.getBuildLocation()))
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
	for (auto point : sorted)
	{
	std::cerr << Distance2D(point, blinkerBot.Observation()->GetStartLocation()) << std::endl;
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
	std::cerr << geysers.size() << " geysers found" << std::endl;
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
	return (*availableBases.begin()).getBuildLocation();
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

void BaseManager::findBases()
{
	std::vector<std::vector<const Unit *>> allMineralLines = findMineralLines();
	std::set<const Unit *> allGeysers = findGeysers();

	const int MOVEDIST = 2;

	for (auto line : allMineralLines)
	{
		Point2D baseLocation;
		std::set<const Unit *> mineralLine;
		std::set<const Unit *> geysers;

		float averageX = 0;
		float averageY = 0;

		//first we locate the geysers associated with the base and calculate an average point between them
		for (auto geyser : allGeysers)
		{
			if (Distance2D(geyser->pos, (*line.begin())->pos) < 15)
			{
				averageX += geyser->pos.x;
				averageY += geyser->pos.y;

				//put this in the set of geysers we're going to construct the base with
				geysers.insert(geyser);
			}
		}
		averageX = averageX / 2;
		averageY = averageY / 2;

		Point2D gasAverage = Point2D(averageX, averageY);
		baseLocation = gasAverage;

		averageX = 0;
		averageY = 0;

		//next we calculate an average point between the mineral lines
		for (auto mineral : line)
		{
			averageX += mineral->pos.x;
			averageY += mineral->pos.y;

			//add the mineral to the set of minerals we're going to construct the base with
			mineralLine.insert(mineral);
		}
		averageX = averageX / line.size();
		averageY = averageY / line.size();

		Point2D mineralAverage = Point2D(averageX, averageY);

		//depending on the relationship between the average mineral point and average gas point, we adjust the build location 
		if (gasAverage.x == mineralAverage.x) //horizontal base
		{
			if (gasAverage.y > mineralAverage.y) //this means that the minerals above
			{
				baseLocation = Point2D(baseLocation.x, baseLocation.y + MOVEDIST);
			}
			else if (gasAverage.y < mineralAverage.y) // the gas is above
			{
				baseLocation = Point2D(baseLocation.x, baseLocation.y - MOVEDIST);
			}
		}
		else if (gasAverage.y == mineralAverage.y) //vertical base
		{
			if (gasAverage.x > mineralAverage.x) //this means that the minerals are to the left
			{
				baseLocation = Point2D(baseLocation.x + MOVEDIST, baseLocation.y);
			}
			if (gasAverage.x < mineralAverage.x) // the gas is to the left
			{
				baseLocation = Point2D(baseLocation.x - MOVEDIST, baseLocation.y);
			}
		}
		else if ((gasAverage.x > mineralAverage.x) && (gasAverage.y > mineralAverage.y)) //top left
		{
			baseLocation = Point2D(baseLocation.x + MOVEDIST, baseLocation.y + MOVEDIST);
		}
		else if ((gasAverage.x > mineralAverage.x) && (gasAverage.y < mineralAverage.y)) //bottom left
		{
			baseLocation = Point2D(baseLocation.x + MOVEDIST, baseLocation.y - MOVEDIST);
		}
		else if ((gasAverage.x < mineralAverage.x) && (gasAverage.y > mineralAverage.y)) //top right
		{
			baseLocation = Point2D(baseLocation.x - MOVEDIST, baseLocation.y + MOVEDIST);
		}
		else if ((gasAverage.x < mineralAverage.x) && (gasAverage.y < mineralAverage.y)) //bottom right
		{
			baseLocation = Point2D(baseLocation.x - MOVEDIST, baseLocation.y - MOVEDIST);
		}

		bases.push_back(Base(mineralLine, geysers, baseLocation));
	}

	/*now we've found all the bases, let's use them to make a vector sorted in order of distance from our main so that
	we know where to expand to*/
	std::vector<Base> unsortedBases;
	for (auto base : bases)
	{
		//when we find our main base, set the unit pointer rather than adding it to the available bases
		if (Distance2D(base.getBuildLocation(), blinkerBot.Observation()->GetStartLocation()) < 4)
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

void Base::setTownhall(const Unit *unit)
{
	this->townhall = unit;
}

void BaseManager::addBase(const Unit *unit)
{
	for (auto base : bases)
	{
		if (Distance2D(base.getBuildLocation(), unit->pos) < 5)
		{
			bool alreadyFound = false;
			for (auto ourBase : ourBases)
			{
				if (Distance2D(base.getBuildLocation(), ourBase.getBuildLocation()) < 5)
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