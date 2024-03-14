#include "Actor.h"
#include "StudentWorld.h"
using namespace std;

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp
Actor::Actor(StudentWorld* sw, int imageID, double startX, double startY, int startDirection = GraphObject::none)
	:GraphObject(imageID, startX, startY, startDirection), m_alive(true), m_sw(sw)
{

}

void Actor::remove(bool givePoints)
{
	if(givePoints)
		getStudentWorld()->increaseScore(holdsPoints());
	m_alive = false;
}
void Actor::kill()
{
	getStudentWorld()->increaseScore(holdsPoints());
	m_alive = false;
}
bool Actor::isAlive()
{
	return m_alive;
}
void Actor::revive(double x, double y)
{
	moveTo(x, y);
	setVisible(true);
	m_alive = true;
}
StudentWorld* Actor::getStudentWorld()
{
	return m_sw;
}

pair<double, double> Actor::newPos(double x, double y, int direction)
{
	switch (direction)
	{
	case up:
		y += 1;
		break;
	case down:
		y -= 1;
		break;
	case left:
		x -= 1;
		break;
	case right:
		x += 1;
		break;
	}
	return { x,y };
}

Wall::Wall(StudentWorld* sw, double x, double y)
	:Actor(sw, IID_WALL, x, y)
{
	setVisible(true);
};

Exit::Exit(StudentWorld* sw, double x, double y)
	: Actor(sw, IID_EXIT, x, y)
{
	setVisible(false);
}

void Exit::doSomething()
{
	StudentWorld* sw = getStudentWorld();
	shared_ptr<Player> player = sw->getPlayer();
	if (player)
	{
		if (getX() == player->getX() && getY() == player->getY() && isVisible())
		{
			sw->increaseScore(2000);
			sw->increaseScore(sw->getBonus());
			sw->setLevelComplete();
		}
	}
}

Pit::Pit(StudentWorld* sw, double x, double y)
	: Actor(sw, IID_PIT, x, y)
{
	setVisible(true);
}

void Pit::doSomething()
{
	if (isAlive())
	{
		StudentWorld* sw = getStudentWorld();
		if (sw->isActorAt(getX(), getY()))
		{
			vector<shared_ptr<Actor>> actors = sw->getActorsAt(getX(), getY()); //TODO: Might get itself here
			for (auto actor : actors)
			{
				if (actor->isPushable())
				{
					remove(false);
					actor->remove(false);
				}
			}
		}
	}
}

Marble::Marble(StudentWorld* sw, double x, double y)
	: Actor(sw, IID_MARBLE, x, y), m_hitPoints(10)
{
	setVisible(true);
}


void Marble::getHit(int points)
{
	m_hitPoints = m_hitPoints - points;
	if (m_hitPoints <= 0)
		remove(false);
}

bool Marble::push(int direction)
{
	StudentWorld* sw = getStudentWorld();
	pair<double, double> updatedMarblePos = newPos(getX(), getY(), direction);
	if (!sw->isWithinBounds(updatedMarblePos.first, updatedMarblePos.second))
	{
		return false;
	}
	if (!sw->isObstructed(updatedMarblePos.first, updatedMarblePos.second))
	{
		moveTo(updatedMarblePos.first, updatedMarblePos.second);
		return true;
	}
	else
	{
		vector<shared_ptr<Actor>> actors = sw->getActorsAt(updatedMarblePos.first, updatedMarblePos.second);
		for (auto actor : actors)
		{
			if (actor->isSurfaceLeveled() && !actor->isExit())
			{
				moveTo(updatedMarblePos.first, updatedMarblePos.second);
				return true;
			}
		}
		return false;
	}

}

Pea::Pea(StudentWorld* sw, double x, double y, int direction)
	: Actor(sw, IID_PEA, x, y, direction)
{
	setVisible(true);
}

void Pea::doSomething()
{
	if (!isAlive())
	{
		return;
	}
	StudentWorld* sw = getStudentWorld();
	double x = getX();
	double  y = getY();
	bool hit = false;
	//Checks if current position is Obstructed
	if (!sw->isWithinBounds(x, y))
		return;
	if (sw->isObstructed(x, y)) //isObstructed only loooks for Items that can Obstruct
	{
		vector<shared_ptr<Actor>> actors = sw->getActorsAt(x, y);
		for (auto actor : actors)
		{
			if (actor->isSurfaceLeveled())
			{
				break;
			}
			if (actor->isKillable())
			{
				shared_ptr<Character> character = dynamic_pointer_cast<Character>(actor);
				shared_ptr<Marble> marble = dynamic_pointer_cast<Marble>(actor);
				if (character)
				{
					character->getHit(2);
				}
				else if (marble)
				{
					marble->getHit(2);
				}
			}
			hit = true;
			break;
		}
	}
	if (hit)
	{
		remove(false);

	}
	else
	{
		pair<double, double> updatedPos = newPos(x, y, getDirection());
		moveTo(updatedPos.first, updatedPos.second);
	}
}

Goodie::Goodie(StudentWorld* sw, int imageID, double x, double y, int points)
	: Actor(sw, imageID, x, y), m_valuePoints(points)
{

}

void Goodie::doSomething()
{
	if (isAlive())
	{
		StudentWorld* sw = getStudentWorld();
		shared_ptr<Player> player = sw->getPlayer();
		if (player && (getX() == player->getX() && getY() == player->getY()))
		{
			getPicked(player);
		}
		// TOOD: Get Picked by a Thief Bot
	}
}

void Goodie::getStolen()
{
	setVisible(false);
	remove(false);
}

RestoreHealth::RestoreHealth(StudentWorld* sw, double x, double y)
	:Goodie(sw, IID_RESTORE_HEALTH, x, y, 500)
{
	setVisible(true);
}

void RestoreHealth::getPicked(std::shared_ptr<Player> player)
{
	StudentWorld* sw = getStudentWorld();
	player->restoreHealth();
	remove(true);
	sw->playSound(SOUND_GOT_GOODIE);
}

ExtraLife::ExtraLife(StudentWorld* sw, double x, double y)
	:Goodie(sw, IID_EXTRA_LIFE, x, y, 1000)
{
	setVisible(true);
}

void ExtraLife::getPicked(std::shared_ptr<Player> player)
{
	StudentWorld* sw = getStudentWorld();
	sw->incLives();
	remove(true);
	sw->playSound(SOUND_GOT_GOODIE);
}

Ammo::Ammo(StudentWorld* sw, double x, double y)
	:Goodie(sw, IID_AMMO, x, y, 100)
{
	setVisible(true);
}

void Ammo::getPicked(shared_ptr<Player> player)
{
	StudentWorld* sw = getStudentWorld();
	player->incrPeas(20);
	remove(true);
	sw->playSound(SOUND_GOT_GOODIE);
}

Crystal::Crystal(StudentWorld* sw, double x, double y)
	:Goodie(sw, IID_CRYSTAL, x, y, 50)
{
	setVisible(true);
}

void Crystal::getPicked(shared_ptr<Player> player)
{
	StudentWorld* sw = getStudentWorld();
	sw->reduceCrystal();
	remove(true);
	sw->playSound(SOUND_GOT_GOODIE);
}


Character::Character(StudentWorld* sw, int imageID, double x, double y, int startDirection, int hitPoints, int valuePoints = 0)
	: Actor(sw, imageID, x, y, startDirection), m_hitPoints(hitPoints), m_valuePoints(valuePoints)
{

}


int Character::getHitPoints()
{
	return m_hitPoints;
}

void Character::getHit(int points)
{
	if (isPlayer())
		getStudentWorld()->playSound(SOUND_PLAYER_IMPACT);
	else
		getStudentWorld()->playSound(SOUND_ROBOT_IMPACT);
	m_hitPoints = m_hitPoints - points;
	if (m_hitPoints <= 0)
	{
		if (canStealGoodies())
		{
 			dropGoodies();
		}
		kill();
	}
}

bool Character::shouldShoot()
{
	int direction = getDirection();
	double X = getX();
	double Y = getY();

	StudentWorld* sw = getStudentWorld();
	shared_ptr<Player> player = sw->getPlayer();
	double playerX = player->getX();
	double playerY = player->getY();


	bool playerInLine = false;
	bool obstructionExists = false;
	// Same Columns
	if (X == playerX)
	{
		if (direction == up && playerY > Y)
		{
			playerInLine = true;
			obstructionExists = sw->isPathObstructed(X, Y + 1, playerX, playerY - 1);
		}
		else if (direction == down && playerY < Y)
		{
			playerInLine = true;
			obstructionExists = sw->isPathObstructed(playerX, playerY + 1, X, Y - 1);
		}
	}
	// Same Row
	else if (Y == playerY)
	{
		if (direction == right && playerX > X)
		{
			playerInLine = true;
			obstructionExists = sw->isPathObstructed(X + 1, Y, playerX - 1, playerY);
		}
		else if (direction == left && playerX < X)
		{
			playerInLine = true;
			obstructionExists = sw->isPathObstructed(playerX + 1, playerY, X - 1, Y);
		}
	}

	return (playerInLine && !obstructionExists);
}


bool Character::move(int direction)
{
	pair<double, double> updatedPos = newPos(getX(), getY(), direction);
	StudentWorld* sw = getStudentWorld();
	if (!sw->isWithinBounds(updatedPos.first, updatedPos.second)) {
		return false; // Early exit if the new position is out of bounds
	}
	if (!sw->isObstructed(updatedPos.first, updatedPos.second))
	{
		moveTo(updatedPos.first, updatedPos.second);
		return true;
	}
	else if (isPlayer())
	{
		vector<shared_ptr<Actor>> actors = sw->getActorsAt(updatedPos.first, updatedPos.second);
		for (auto actor : actors)
		{
			if (actor->isPushable() || actor->isExit())
			{
				shared_ptr<Marble> marble = dynamic_pointer_cast<Marble>(actor);
				shared_ptr<Exit> exit = dynamic_pointer_cast<Exit>(actor);
				if (marble)
				{
					if (marble->push(direction))
					{
						moveTo(updatedPos.first, updatedPos.second);
						return true;
					}
				}
				else if (exit)
				{
					moveTo(updatedPos.first, updatedPos.second);
					return true;
				}
			}
		}

	}
	return false;

}

void Character::shoot(int direction)
{
	pair<double, double> updatedPos = newPos(getX(), getY(), direction);
	StudentWorld* sw = getStudentWorld();
	bool isValidPos = sw->isWithinBounds(updatedPos.first, updatedPos.second) && !sw->isObstructed(updatedPos.first, updatedPos.second);
	if (sw->isWithinBounds(updatedPos.first, updatedPos.second))
	{
		if (isPlayer())
		{
			if (getPeas() > 0)
			{
				reducePeas();
			}
			else
			{
				return;
			}
		}
		getStudentWorld()->addPea(make_shared<Pea>(sw, updatedPos.first, updatedPos.second, direction));
		if (isPlayer())
			sw->playSound(SOUND_PLAYER_FIRE);
		else
			sw->playSound(SOUND_ENEMY_FIRE);
	}

}

RageBot::RageBot(StudentWorld* sw, double x, double y, int startDirection)
	: Character(sw, IID_RAGEBOT, x, y, startDirection, 10, 100)
{
	setVisible(true);
	//Setting Rest Time
	int ticks = (28 - sw->getLevel()) / 4;
	if (ticks < 3)
		ticks = 3;
	m_maxRestTime = ticks;
	m_restTime = ticks;
}

void RageBot::doSomething()
{
	int direction = getDirection();
	if (m_restTime > 0)
	{
		m_restTime--;
		return;
	}
	else
	{
		m_restTime = m_maxRestTime;
		if (shouldShoot())
		{
			shoot(direction);
		}
		else
		{
			if (!move(direction))
			{
				setDirection(direction + 180);
			}
		}

	}
}

ThiefBot::ThiefBot(StudentWorld* sw, int imageID, double x, double y, botType type)
	:Character(sw, imageID, x, y, GraphObject::right, 
		(type == botType::mean) ? 8 : 5, 
		(type == botType::mean) ? 20 : 10), m_type(type), m_goodie(nullptr)
{
	setVisible(true);
	//Setting Rest Time
	int ticks = (28 - sw->getLevel()) / 4;
	if (ticks < 3)
		ticks = 3;
	cout << ticks << endl;
	m_maxRestTime = ticks;
	m_restTime = ticks;
	m_distanceBeforeTurning = randInt(1, 6);
}

void ThiefBot::doSomething()
{
	int direction = getDirection();
	if (m_restTime > 0)
	{
		m_restTime--;
		return;
	}
	else
	{
		m_restTime = m_maxRestTime;
		double x = getX();
		double y = getY();
		StudentWorld* sw = getStudentWorld();
		vector<shared_ptr<Actor>> actors = sw->getActorsAt(x, y);
		if (!actors.empty())
		{
			for (auto actor : actors)
			{
				//if (actor->isPickable() && m_goodie == nullptr && randInt(1, 10) == 1) TODO
				if (m_goodie == nullptr)
				{
					shared_ptr<Goodie> goodie = dynamic_pointer_cast<Goodie> (actor);
					if (goodie)
					{
						goodie->getStolen();
						m_goodie = goodie;
						return;
					}
				}
			}
		}
		
		if (m_distanceBeforeTurning > 0 && move(direction))
		{
			m_distanceBeforeTurning--;
			return;
		}
		else
		{
			m_distanceBeforeTurning = randInt(1, 6);
			int randomDirection = randInt(0, 3) * 90;
			for (int i = 0; i < 4; i++)
			{
				randomDirection = randomDirection + (i*90);
				setDirection(randomDirection);
				if (move(direction))
					return;
			}
		}
	}
	

}

void ThiefBot::dropGoodies()
{
	if (m_goodie)
	{
		getStudentWorld()->addGoodie(m_goodie);
		m_goodie->revive(getX(),getY());
	}
}



std::shared_ptr<Goodie> ThiefBot::getGoodie()
{
	return m_goodie;
}

Player::Player(StudentWorld* sw, double x, double y)
	: Character(sw, IID_PLAYER, x, y, GraphObject::right, 20), m_peas(20)
{
	setVisible(true);
}

void Player::doSomething()
{
	if (!isAlive())
	{
		return;
	}
	int ch;
	if (getStudentWorld()->getKey(ch))
	{
		switch (ch)
		{
		case KEY_PRESS_ESCAPE:
			kill();
			break;
		case KEY_PRESS_TAB:
			// CHEAT
			break;
		case KEY_PRESS_SPACE:
			shoot(getDirection());
			break;
		case KEY_PRESS_LEFT:
			setDirection(left);
			move(left);
			break;
		case KEY_PRESS_RIGHT:
			setDirection(right);
			move(right);
			break;
		case KEY_PRESS_UP:
			setDirection(up);
			move(up);
			break;
		case KEY_PRESS_DOWN:
			setDirection(down);
			move(down);
			break;
		default:
			cerr << "INVALID KEY PRESS" << endl;
			break;
		}
	}

}

int Player::getPeas()
{
	return m_peas;
}

void Player::reducePeas()
{
	m_peas--;
}

void Player::incrPeas(int peas)
{
	m_peas += peas;
}

void Player::restoreHealth()
{
	m_hitPoints = 20;
}

void Player::cheat()
{
	//TODO: Cheat
}

ThiefBotFactory::ThiefBotFactory(StudentWorld* sw, double x, double y, factorytype type)
	:Actor(sw, IID_ROBOT_FACTORY, x, y), m_factoryType(type)
{
	setVisible(true);
}

void ThiefBotFactory::doSomething()
{
	StudentWorld* sw = getStudentWorld();
	double currentX = getX();
	double currentY = getY();
	int count = 0;
	for (double x = currentX - 3; x < currentX + 3; x++)
	{
		for (double y = currentY - 3; y < currentY + 3; y++)
		{
			if (sw->isWithinBounds(x, y))
			{
				vector <shared_ptr<Actor>> actors = sw->getActorsAt(x, y);
				if (!actors.empty())
				{
					for (auto actor : actors)
					{
						if (dynamic_pointer_cast<ThiefBot>(actor))
						{
							count++;
						}
					}
				}
			}
		}
	}
	if (count < 1)
	{
		// 1 in 50 chance
		if (m_factoryType == regular && (randInt(1, 50) == 1))
		{
			shared_ptr<ThiefBot> bot = make_shared<ThiefBot>(sw, IID_THIEFBOT, currentX, currentY, ThiefBot::botType::regular);
			sw->addThiefBot(bot);
		}
		else if (m_factoryType == mean && (randInt(1, 50) == 1))
		{
			shared_ptr<ThiefBot> bot = make_shared<ThiefBot>(sw, IID_MEAN_THIEFBOT, currentX, currentY, ThiefBot::botType::mean);
			sw->addThiefBot(bot);
		}
	}
}


