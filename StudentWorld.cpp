#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
	: GameWorld(assetPath), m_crystals(0), m_player(nullptr), m_levelComplete(false), m_bonus(1000)
{
	m_displayText = "";
}

int StudentWorld::init()
{
	// Get Level
	const string levelString = "0" + to_string(GameWorld::getLevel()); //TODO: Assuming Level < 10
	Level level(GameWorld::assetPath());

	// Load Level File
	const string levelFile = "level" + levelString + ".txt"; // depends on if level is 1 or 2 digit
	const Level::LoadResult result = level.loadLevel(levelFile);

	if (result == Level::load_fail_file_not_found) {
		cerr << "Cannot find " << levelFile << " data file" << endl;
		return GWSTATUS_LEVEL_ERROR;
	}
	else if (result == Level::load_fail_bad_format)
	{
		cerr << "Your level was improperly formatted" << endl;
		return GWSTATUS_LEVEL_ERROR;
	}
	else if (result == Level::load_success)
	{
		for (double x = 0; x < VIEW_HEIGHT; x++)
		{
			for (double y = 0; y < VIEW_WIDTH; y++)
			{
				addActor(level.getContentsOf(int(x), int(y)), x, y);
			}
		}
	}
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::addActor(Level::MazeEntry actor, double x, double y)
{
	switch (actor)
	{
	case Level::wall:
	{
		m_actors.push_back(make_shared<Wall>(this, x, y));
		break;
	}
	case Level::exit:
	{
		m_actors.push_back(make_shared<Exit>(this, x, y));
		break;
	}
	case Level::pit:
	{
		m_actors.push_back(make_shared<Pit>(this, x, y));
		break;
	}
	case Level::marble:
	{
		m_actors.push_back(make_shared<Marble>(this, x, y));
		break;
	}
	case Level::extra_life:
	{
		m_actors.push_back(make_shared<ExtraLife>(this, x, y));
		break;
	}
	case Level::restore_health:
	{
		m_actors.push_back(make_shared<RestoreHealth>(this, x, y));
		break;
	}
	case Level::ammo:
	{
		m_actors.push_back(make_shared<Ammo>(this, x, y));
		break;
	}
	case Level::crystal:
	{
		m_actors.push_back(make_shared<Crystal>(this, x, y));
		m_crystals++;
		break;
	}
	case Level::player:
	{
		shared_ptr<Player> player = make_shared<Player>(this, x, y);
		m_actors.push_back(player);
		m_player = player;
		break;
	}
	case Level::vert_ragebot:
	{
		m_actors.push_back(make_shared<RageBot>(this, x, y, GraphObject::down));
		break;
	}
	case Level::horiz_ragebot:
	{
		m_actors.push_back(make_shared<RageBot>(this, x, y, GraphObject::right));
		break;
	}
	case Level::thiefbot_factory:
	{
		m_actors.push_back(make_shared<ThiefBotFactory>(this, x, y, ThiefBotFactory::factorytype::regular));
		break;
	}
	case Level::mean_thiefbot_factory:
	{
		m_actors.push_back(make_shared<ThiefBotFactory>(this, x, y, ThiefBotFactory::factorytype::mean));
		break;
	}
	default:
	{
		break;
	}
	}

}

void StudentWorld::exposeExit()
{
	for (auto& actor : m_actors)
	{
		if (actor->isExit())
		{
			actor->setVisible(true);
		}
	}
}

void StudentWorld::addThiefBot(std::shared_ptr<ThiefBot> thiefbot)
{
	m_actorsQueue.push_back(thiefbot);
}

void StudentWorld::addPea(shared_ptr<Pea> pea)
{
	m_actorsQueue.push_back(pea);
}

void StudentWorld::addGoodie(std::shared_ptr<Goodie> goodie)
{
	m_actorsQueue.push_back(goodie);
}

void StudentWorld::moveQueue()
{
	for (auto actor : m_actorsQueue)
	{
		m_actors.push_back(actor);
	}
	m_actorsQueue.clear();
}

void StudentWorld::reduceCrystal()
{
	m_crystals--;
	if (m_crystals <= 0)
	{
		exposeExit();
	}
}

void StudentWorld::setLevelComplete()
{
	m_levelComplete = true;
}

bool StudentWorld::isActorAt(double x, double y)
{
	for (auto actor : m_actors)
	{
		if (actor->getX() == x && actor->getY() == y)
		{
			return true;
		}
	}
	return false;
}

vector<shared_ptr<Actor>> StudentWorld::getActorsAt(double x, double y)
{
	vector<shared_ptr<Actor>> result;
	for (auto actor : m_actors)
	{
		if (actor->getX() == x && actor->getY() == y)
		{
			result.push_back(actor);
		}
	}
	return result;
}

std::shared_ptr<Player> StudentWorld::getPlayer()
{
	return m_player;
}

int StudentWorld::getCrystalsLeft()
{
	return m_crystals;
}

int StudentWorld::getBonus()
{
	return m_bonus;
}


int StudentWorld::move()
{
	// TODO: here actors like pea and thiefbots and goodies maybe added or removed during doSomething of other actors. need to call doSomething for the added ones without breaking the loop
	updateDisplaytext();
	for (auto actor : m_actors)
	{
		if (actor->isAlive())
		{
			actor->doSomething();
			if (!m_player->isAlive())
			{
				playSound(SOUND_PLAYER_DIE);
				decLives();
				return GWSTATUS_PLAYER_DIED;
			}
			if (m_levelComplete)
			{
				return GWSTATUS_FINISHED_LEVEL;
			}
		}
	}
	moveQueue();
	reduceLevelBonusByOne();
	removeDeadGameObjects();


	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::updateDisplaytext()
{
	string seperator = "  ";
	m_displayText = "Bonus: " + to_string(m_bonus) + seperator
		+ "Score: " + to_string(getScore()) + seperator
		+ "Ammo: " + to_string(m_player->getPeas()) + seperator
		+ "Hit Points " + to_string(m_player->getHitPoints());
	setGameStatText(m_displayText);
}

void StudentWorld::removeDeadGameObjects()
{
	auto newEnd = remove_if(m_actors.begin(), m_actors.end(), [](shared_ptr<Actor> actor) { return !actor->isAlive(); }); //TODO: Maybe exclude Player?
	m_actors.erase(newEnd, m_actors.end());
}

void StudentWorld::reduceLevelBonusByOne()
{
	if (m_bonus > 0)
	{
		m_bonus--;
	}
}


void StudentWorld::cleanUp()
{
	m_player = nullptr;
	m_crystals = 0;
	m_levelComplete = false;
	m_bonus = 1000;
	m_actors.clear();
}

bool StudentWorld::isWithinBounds(double x, double y)
{
	if ((x >= 0 && x < VIEW_HEIGHT) && (y >= 0 && y < VIEW_WIDTH))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool StudentWorld::isObstructed(double x, double y)
{
	if (isActorAt(x, y))
	{
		vector<shared_ptr<Actor>> actors = getActorsAt(x, y);
		for (auto actor : actors)
		{
			if (actor && actor->blocksMovement())
			{
				return true;
			}
		}
	}
	return false;
}

bool StudentWorld::isPathObstructed(double startX, double startY, double endX, double endY)
{
	if (startX == endX)
	{
		double x = startX;
		for (double y = startY; y <= endY; y++)
		{
			if (isObstructed(x, y))
			{
				vector<shared_ptr<Actor>> actors = getActorsAt(x, y);
				for (auto actor : actors)
				{
					if (!actor->isSurfaceLeveled())
						return true;
				}
			}
		}
		return false;
	}
	else if (startY == endY)
	{
		double y = startY;
		for (double x = startX; x <= endX; x++)
		{
			if (isObstructed(x, y))
			{
				vector<shared_ptr<Actor>> actors = getActorsAt(x, y);
				for (auto actor : actors)
				{
					if (!actor->isSurfaceLeveled())
						return true;
				}
			}
		}
		return false;
	}
	return true;
}


