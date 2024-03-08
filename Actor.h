#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
class StudentWorld;

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp
class Actor : public GraphObject
{
public:
	//enum actorType {
	//	exit, player, ragebot, thiefbot, thiefbot_factory, mean_thiefbot_factory, wall, marble, pit,
	//	crystal, restore_health_goodie, extra_life_goodie, ammo_goodie, unknown
	//};
	Actor(StudentWorld* sw, int imageID, double startX, double startY, int startDirection);
    
	virtual void doSomething() { return; }
	virtual bool blocksMovement() { return false; }
	virtual bool isPickable() { return false; }
	virtual bool isMovable() { return false;  }
	virtual bool isPushable() { return false; }
	virtual bool isKillable() { return false; }
	virtual int holdsPoints() { return 0; }
	virtual bool isExit() { return false; }

	void remove();
	void kill();
	bool isAlive();
	StudentWorld* getStudentWorld();
	
	std::pair<double,double> newPos(double x, double y, int direction);

private:
	bool m_alive;
	StudentWorld* m_sw;
    
};


class Wall : public Actor {
public:
	Wall(StudentWorld* sw, double x, double y);
	virtual bool blocksMovement() { return true; }
};


class Exit : public Actor {
public:
    Exit(StudentWorld* sw, double x, double y);
	virtual bool isExit() { return true; }
	virtual bool blocksMovement() { return true; }
	virtual void doSomething();
};

class Pit : public Actor {
public:
    Pit(StudentWorld* sw, double x, double y); 
	virtual bool blocksMovement() { return true; } //ODO: Will cause issues for Peas passing (Implement isPit() maybe?)
	virtual void doSomething();
};


class Marble : public Actor {
public:
	Marble(StudentWorld* sw, double x, double y);
	virtual bool isPushable() { return true; }
	virtual bool isKillable() { return true; }
	virtual bool blocksMovement() { return true; }
	void getHit(int points);
	bool push(int direction);
private:
	int m_hitPoints;

};

class Pea : public Actor {
public:
	Pea(StudentWorld* sw, double x, double y, int direction);
	virtual bool isMovable() { return true; }
	virtual void doSomething();
};


// CHARACTERS

class Character : public Actor {
public:
	Character(StudentWorld* sw, int imageID, double x, double y, int startDirection, int hitPoints, int valuePoints);
	virtual bool blocksMovement() { return true; }
	virtual bool isMovable() { return true; }
	virtual bool isKillable() { return true; }
	virtual bool isPlayer() { return false; }
	virtual int getPeas() { return 0; }
	virtual int holdsPoints() { return m_valuePoints; }

	virtual void reducePeas() { return; }
	virtual void incrPeas(int peas) { return; }

	int getHitPoints();
	void getHit(int points);
	bool shouldShoot();
	
private:
	int m_valuePoints;
protected:
	int m_hitPoints;
	bool move(int direction);
	void shoot(int direction);
};

class RageBot : public Character {
public:
	enum pathType {vertical, horizontal};
	RageBot(StudentWorld* sw, double x, double y, int startDirection);
	virtual void doSomething();
private:
	int m_maxRestTime;
	int m_restTime;
	pathType m_path;
};

class Player : public Character {
public:
	Player(StudentWorld* sw, double x, double y);
	virtual int holdsPoints() { return 0; }
	virtual void doSomething();
	virtual bool isPlayer() { return true; }
	virtual int getPeas();
	virtual void reducePeas();
	virtual void incrPeas(int peas);
	void restoreHealth();
private:
	void cheat();
	int m_peas;
};


// GOODIES 

class Goodie : public Actor {
public:
	Goodie(StudentWorld* sw, int imageID, double x, double y, int points);
	virtual bool isPickable() { return true; }
	virtual int holdsPoints() { return m_valuePoints; }
	virtual void getPicked(std::shared_ptr<Player> player) { return; }
	//virtual void getPicked(std::shared_ptr<ThiefBot> thiefbot) { return; }
	virtual void doSomething();

private:
	int m_valuePoints;
};

class RestoreHealth : public Goodie {
public:
	RestoreHealth(StudentWorld* sw, double x, double y);
	virtual void getPicked(std::shared_ptr<Player> player);
};

class ExtraLife : public Goodie {
public:
	ExtraLife(StudentWorld* sw, double x, double y);
	virtual void getPicked(std::shared_ptr<Player> player);

};

class Ammo : public Goodie {
public:
	Ammo(StudentWorld* sw, double x, double y);
	virtual void getPicked(std::shared_ptr<Player> player);
};

class Crystal : public Goodie {
public:
	Crystal(StudentWorld* sw, double x, double y);
	virtual void getPicked(std::shared_ptr<Player> player);
};




//class ThiefBotFactory : public Actor {
//public:
//    ThiefBotFactory(double x, double y);
//};
//
//class MeanThiefBotFactory : public Actor {
//public:
//    MeanThiefBotFactory(double x, double y);
//};
//
//



#endif // ACTOR_H_
