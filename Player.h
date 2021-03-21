/*  Nathan W Brown
brow1982@umn.edu
EE1301 - Fall 2016
Final Project	*/

#include <iostream>
#include <string>

#ifndef PLAYER_H
#define PLAYER_H

class Player {
public: //function declarations
	//default constructor (set member vars to null state)
	Player();	

	//overload constructor
	Player(std::string, std::string, int, int, int, int, int, int, int, int);			

	//destructor
	~Player();

	//accessor functions (return member variables)
	std::string getName() const;
			//get name of player
	std::string getBotName() const;
	int getWeapon1() const;
	int getWeapon2() const;
	int getHandling() const;
	int getAim() const;
	int getSpeed() const;
	int getArmor() const;
	int getHealth() const;
	int getHeatCap() const;

	//mutator functions (edit member variables)
	void setName(std::string);
			//set name of player @param string - newName
	void setBotName(std::string);
			//set name of the battlebot @param string - newBotName
	void setWeapon1(int);
	void setWeapon2(int);
	void setHandling(int);
	void setAim(int);
	void setSpeed(int);
	void setArmor(int);
	void setHealth(int);
	void setHeatCap(int);

private: //member variables
	std::string Name;
	std::string BotName;
	int Weapon1, Weapon2, Handling, Aim, Speed, Armor, Health, HeatCap;

};

#endif