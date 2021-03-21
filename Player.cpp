/*  Nathan W Brown
brow1982@umn.edu
EE1301 - Fall 2016
Final Project	*/

//function definitions

#include "Player.h"

Player::Player() {
	Name = "";
	BotName = "";
	Weapon1 = NULL;
	Weapon2 = NULL;
	Handling = NULL;
	Aim = NULL;
	Speed = NULL;
	Armor = NULL;
	Health = NULL;
	HeatCap = NULL;
}

Player::Player(std::string newName, std::string newBotName, int newWeapon1, int newWeapon2, int newHandling, int newAim, int newSpeed, int newArmor, int newHealth, int newHeatCap) {
	Name = newName;
	BotName = newBotName;
	Weapon1 = newWeapon1;
	Weapon2 = newWeapon2;
	Handling = newHandling;
	Aim = newAim;
	Speed = newSpeed;
	Armor = newArmor;
	Health = newHealth;
	HeatCap = newHeatCap;
}

Player::~Player() {
}

std::string Player::getName() const{
	return Name;
}

std::string Player::getBotName() const {
	return BotName;
}

int Player::getWeapon1() const {
	return Weapon1;
}

int Player::getWeapon2() const {
	return Weapon2;
}

int Player::getHandling() const {
	return Handling;
}

int Player::getAim() const {
	return Aim;
}

int Player::getSpeed() const {
	return Speed;
}

int Player::getArmor() const {
	return Armor;
}

int Player::getHealth() const {
	return Health;
}

int Player::getHeatCap() const {
	return HeatCap;
}

void Player::setName(std::string newName) {
	Name = newName;
}

void Player::setBotName(std::string newBotName) {
	BotName = newBotName;
}

void Player::setWeapon1(int newWeapon1) {
	Weapon1 = newWeapon1;
}

void Player::setWeapon2(int newWeapon2) {
	Weapon2 = newWeapon2;
}

void Player::setHandling(int newHandling) {
	Handling = newHandling;
}

void Player::setAim(int newAim) {
	Aim = newAim;
}

void Player::setSpeed(int newSpeed) {
	Speed = newSpeed;
}

void Player::setArmor(int newArmor) {
	Armor = newArmor;
}

void Player::setHealth(int newHealth) {
	Health = newHealth;
}

void Player::setHeatCap(int newHeatCap) {
	HeatCap = newHeatCap;
}