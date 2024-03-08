#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Level.h"
#include "Actor.h"
#include <string>
#include <vector>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetPath);
	virtual int init();
	virtual int move();
	virtual void cleanUp();

	bool isWithinBounds(double x, double y);
	bool isObstructed(double x, double y);
	bool isActorAt(double x, double y);
	std::vector <std::shared_ptr<Actor>> getActorsAt(double x, double y);
	std::shared_ptr<Player> getPlayer();
	int getCrystalsLeft();
	int getBonus();

	void addPea(double x, double y, int direction);
	void reduceCrystal();
	void setLevelComplete();

private:
	std::vector<std::shared_ptr<Actor>> m_actors;
	int m_crystals;
	int m_bonus;
	std::shared_ptr<Player> m_player;
	std::string m_displayText;
	bool m_levelComplete;
	// From GameWorld.h -> use Getters
	//int				m_lives;
	//int				m_score;
	//int				m_level;

	void addActor(Level::MazeEntry actor, double x, double y);
	void exposeExit();
	
	void updateDisplaytext();
	void removeDeadGameObjects();
	void reduceLevelBonusByOne();
};

#endif // STUDENTWORLD_H_
