/*  Nathan W Brown
brow1982@umn.edu
EE1301 - Fall 2016
Final Project	*/

#include <ctime>
#include <iostream>
#include <fstream>
#include <string>

#include "Player.h"

#define NUM_WEAPONS 10										//0 = MG, 1 = SL, 2 = ML, 3 = LL, 4 = PL, 5 = 90mm, 6=150mm, 7=gauss, 8=PPC, 9=LG
#define NUM_LOCATIONS 3										//0 = open desert, 1 = dense forest, 2 = icy ridge
#define TOHIT_BASE 13									    //base chance to hit is 40% 13-20 on 1-20

int selectionChecker();											    //selection function prototypes
int mainMenuFunction(int);
int botSelectFunction(int);
int enemybotSelectFunction(int);
int locationSelectFunction(int);

int newGame();														//main menu function prototypes
void continueGame();
void returnHowtoplay();

int battleFunction(int,int,int);									//the combat function prototypes
int enemyAI(int, int);

struct Weapon {												//declare a struct to hold information about each weapon
	std::string weaponName;
	int weaponRange;
	int weaponDamage;
	int weaponHeat;
};

struct Location {											//declare a struct to hold information about each location
	std::string locationName;
	int startRange;
	int heatPenalty;
	int speedPenalty;
};

struct Weapon weapons[NUM_WEAPONS];							//declare a global array of type Weapon to hold information for all weapons and locations used in the game
struct Location locations[NUM_LOCATIONS];

Player CurrentPlayer, CurrentEnemy;							//declare global player classes

int main()
{
	int userMenuSelection, userMenuConfirm, battleResult;					//declare main function variables
	int startingLocation, startingRange, startingRound;						//declare main function variables to pass to the battle function
	bool continueLoad=false;
	std::string outLine;

	srand((unsigned int)time(NULL));						//seed the RNG

	std::ifstream weaponfile("weapons.txt");				//load weapons file
	for (int i = 0; i < NUM_WEAPONS; i++) {					//fill the global weapon array with all the weapons in the game
		weaponfile >> weapons[i].weaponName >> weapons[i].weaponRange >> weapons[i].weaponDamage >> weapons[i].weaponHeat;
	}
	weaponfile.close();										//close weapon file

	std::ifstream locationfile("locations.txt");			//load locations file
	for (int i = 0; i < NUM_LOCATIONS; i++) {				//fill the global locations array with all the locations in the game
		locationfile >> locations[i].locationName >> locations[i].startRange >> locations[i].heatPenalty >> locations[i].speedPenalty;
	}
	locationfile.close();									//close locations file

	std::ifstream titlefile("title.txt");					//open title file and output the title screen
	if (titlefile.is_open())	{							
		while (getline(titlefile, outLine))
			std::cout << outLine << std::endl;
		titlefile.close();									//close the title file
	}

	MenuSelect:
	userMenuSelection = selectionChecker();					//confirm and check the user selection from main menu for proper input
	
	userMenuConfirm = mainMenuFunction(userMenuSelection);	//pass the confirmed menu selection to execute the menu function
	switch (userMenuConfirm) {
		case 1: 
				startingLocation = newGame();							//start location is returned from the newgame function
				startingRange = locations[startingLocation].startRange;	//get the starting range from the selected location
				startingRound = 0;
				std::cout << "********************Welcome to Battlebot Combat Simulator!*********************" << std::endl;
				break;
		case 2: 
				continueGame();
				continueLoad = true;
				std::cout << "******************Welcome back to Battlebot Combat Simulator!******************" << std::endl;
				break;
		case 3: 
				returnHowtoplay();
				std::cout << "*********Are you ready? 1) NEW GAME 2) CONTINUE 3) HOW TO PLAY 4) EXIT*********" << std::endl;
				goto MenuSelect;
		case 4:
				return 1; //return 1 = good exit state (manual exit)
		default:
				std::cout << std::endl << "Something went wrong!" << std::endl;
				return 2; //error 2, something went wrong in the game setup
	}
	if (continueLoad) {	//seed the main variables for the continue game state
		std::ifstream savedgamestate("savedgamestate.txt");															 //open the saved game state file
		savedgamestate >> startingLocation >> startingRange >> startingRound;										 //acquire the saved game state variables
		savedgamestate.close();																					  	 //close the saved game state file
	}

	battleResult = battleFunction(startingLocation, startingRange, startingRound);
	if (battleResult == 99)
		return 3;													//error 3, something went wrong in the battle function
	else if (battleResult == 0) {
		std::ifstream losefile("losescreen.txt");					//open lose screen file and output the lose screen line by line
		if (losefile.is_open()) {
			while (getline(losefile, outLine))
				std::cout << outLine << std::endl;
			losefile.close();										//close the lose screen file file
		}
	}
	else if (battleResult == 1) {
		std::ifstream winfile("winscreen.txt");						//open the wil screen file and output the win screen line by line
		if (winfile.is_open()) {
			while (getline(winfile, outLine))
				std::cout << outLine << std::endl;
			winfile.close();										//close the win screen file
		}
	}
	else if (battleResult == 2)
		return 0;													//return 0 = good exit state
	else
		return 4;													//error 4, something went wrong in deciding who wins

    return 0;														//return 0 = good exit state (completed successfully)
}

int selectionChecker() {
	int Selection;
	while (!(std::cin >> Selection)) {
		std::cin.clear();
		while (std::cin.get() != '\n')
			continue;
		std::cout << std::endl << "Sorry Dave, I do not understand your input... Please try again: ";
	}
	return Selection;
}

int newGame() {
	std::string newName, outLine;
	int botSelection, enemySelection, locationSelection, botSelectConfirm, enemySelectConfirm, locationSelectConfirm;
	char confirm;

	std::cout << "Welcome to the fight, pilot! Please enter your pilot ID: ";		//get pilot name
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::getline (std::cin, newName);

	NameConfirmation:																//confirm pilot name
	std::cout << "You entered " << newName << ". Is this correct y/n?";
	std::cin >> confirm;
	if (tolower(confirm) == 'y') {
		CurrentPlayer.setName(newName);
	}
	else {
		std::cout << std::endl << "Are you messing with me, pilot? Enter your ID for identity verification: ";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::getline(std::cin, newName);
		goto NameConfirmation;
	}

	//prompt user to select their battlebot
	std::cout << "Welcome " << CurrentPlayer.getName() << "." << std::endl << "We have 3 new bots available, please select your BattleBot: " << std::endl;
	std::ifstream playerbotsfile("playablebots.txt");					//open playable bots file and output them
	if (playerbotsfile.is_open()) {
		while (getline(playerbotsfile, outLine))
			std::cout << outLine << std::endl;
		playerbotsfile.close();											//close the playable bots file
	}

	botSelection = selectionChecker();									//confirm and check the user selection from bot select menu for proper input

	botSelectConfirm = botSelectFunction(botSelection);					//pass the confirmed bot selection to display the bot stats to the user

	switch (botSelectConfirm) {											//fill the player object with the bot stats that are selected
		case 1:
			CurrentPlayer.setBotName("Razor");
			CurrentPlayer.setWeapon1(4); //pulse laser
			CurrentPlayer.setWeapon2(0); //machine gun
			CurrentPlayer.setHandling(-3);
			CurrentPlayer.setAim(3);
			CurrentPlayer.setSpeed(6);
			CurrentPlayer.setArmor(1);
			CurrentPlayer.setHealth(80);
			CurrentPlayer.setHeatCap(60);
			break;
		case 2:
			CurrentPlayer.setBotName("Maul");
			CurrentPlayer.setWeapon1(6); //150mm cannon
			CurrentPlayer.setWeapon2(2); //medium laser
			CurrentPlayer.setHandling(-1);
			CurrentPlayer.setAim(1);
			CurrentPlayer.setSpeed(4);
			CurrentPlayer.setArmor(2);
			CurrentPlayer.setHealth(100);
			CurrentPlayer.setHeatCap(50);
			break;
		case 3:
			CurrentPlayer.setBotName("Ogre");
			CurrentPlayer.setWeapon1(8); //PPC
			CurrentPlayer.setWeapon2(5); //90mm cannon
			CurrentPlayer.setHandling(2);
			CurrentPlayer.setAim(0);
			CurrentPlayer.setSpeed(3);
			CurrentPlayer.setArmor(3);
			CurrentPlayer.setHealth(120);
			CurrentPlayer.setHeatCap(40);
			break;
		default:														//default case returns to main if something went wrong
			std::cout << std::endl << "Something went wrong!" << std::endl;
			return 99;
	}

	//prompt user to select enemy battlebot
	std::cout << std::endl << "Ah, the " << CurrentPlayer.getBotName() << ", good choice " << CurrentPlayer.getName() << "! Now, select your opponent: " << std::endl;
	std::ifstream enemybotsfile("enemybots.txt");						//open enemy selection menu and output it
	if (enemybotsfile.is_open()) {
		while (getline(enemybotsfile, outLine))
			std::cout << outLine << std::endl;
		enemybotsfile.close();											//close the enemy selection menu
	}
	enemySelection = selectionChecker();								//confirm and check the user selection from enemy select menu for proper input
	enemySelectConfirm = enemybotSelectFunction(enemySelection);

	switch (enemySelectConfirm) {										//fill the enemy player object with the bot stats that are selected
		case 1:
			CurrentEnemy.setName("Don Young");
			CurrentEnemy.setBotName("Starwolf");
			CurrentEnemy.setWeapon1(3); //large laser
			CurrentEnemy.setWeapon2(2); //medium laser
			CurrentEnemy.setHandling(1);
			CurrentEnemy.setAim(1);
			CurrentEnemy.setSpeed(4);
			CurrentEnemy.setArmor(2);
			CurrentEnemy.setHealth(60);
			CurrentEnemy.setHeatCap(40);
			break;
		case 2:
			CurrentEnemy.setName("Ren Dorian");
			CurrentEnemy.setBotName("Hound");
			CurrentEnemy.setWeapon1(7); //gauss rifle
			CurrentEnemy.setWeapon2(1); //small laser
			CurrentEnemy.setHandling(-1);
			CurrentEnemy.setAim(1);
			CurrentEnemy.setSpeed(5);
			CurrentEnemy.setArmor(2);
			CurrentEnemy.setHealth(100);
			CurrentEnemy.setHeatCap(50);
			break;
		case 3:
			CurrentEnemy.setName("Vaughn von Blackmore");
			CurrentEnemy.setBotName("Dr.Black");
			CurrentEnemy.setWeapon1(9); //lightning gun
			CurrentEnemy.setWeapon2(5); //90mm cannon
			CurrentEnemy.setHandling(0);
			CurrentEnemy.setAim(2);
			CurrentEnemy.setSpeed(5);
			CurrentEnemy.setArmor(3);
			CurrentEnemy.setHealth(125);
			CurrentEnemy.setHeatCap(50);
			break;
		default:														//default case returns to main if something went wrong
			std::cout << std::endl << "Something went wrong!" << std::endl;
			return 99;
		}

	//prompt user to select the arena location
	std::cout << std::endl << "So, you're going up against " << CurrentEnemy.getBotName() << "?" << std::endl <<
		"It should be a good match in your " << CurrentPlayer.getBotName() << ", " << CurrentPlayer.getName() << "!" << std::endl <<
		"Now, let me know which planet to drop you two on: " << std::endl;
	std::ifstream locationdescriptionfile("locationdescriptions.txt");				//open location description menu and output it
	if (locationdescriptionfile.is_open()) {
		while (getline(locationdescriptionfile, outLine))
			std::cout << outLine << std::endl;
		locationdescriptionfile.close();											//close the location description menu
	}

	locationSelection = selectionChecker();											//confirm and check the user selection from location select menu for proper input
	locationSelectConfirm = locationSelectFunction(locationSelection);				//pass the confirmed bot selection to display the location selections to the user
	switch (locationSelectConfirm) {												//return the selected location to begin the battle!
	case 1:
		return 0;
		break;
	case 2:
		return 1;
		break;
	case 3:
		return 2;
		break;
	default:																		//default case returns to main if something went wrong
		std::cout << std::endl << "Something went wrong!" << std::endl;
		return 99;
	}
}

void continueGame() {
	std::string pilotName, botName;																		 //declare class variables to hold data from the text file
	int savedWeapon1, savedWeapon2, savedHandling, savedAim, savedSpeed, savedArmor, savedHealth, savedHeatCap;
	std::ifstream savedpilot("savedpilot.txt");															 //open the saved pilot file
	std::getline(savedpilot, pilotName);
	savedpilot >> botName >> savedWeapon1 >> savedWeapon2 >> savedHandling >> savedAim >>	 //read txt file into player object
		savedSpeed >> savedArmor >> savedHealth >> savedHeatCap;                  
	savedpilot.close();

	CurrentPlayer.setName(pilotName);
	CurrentPlayer.setBotName(botName);
	CurrentPlayer.setWeapon1(savedWeapon1);
	CurrentPlayer.setWeapon2(savedWeapon2);
	CurrentPlayer.setHandling(savedHandling);
	CurrentPlayer.setAim(savedAim);
	CurrentPlayer.setSpeed(savedSpeed);
	CurrentPlayer.setArmor(savedArmor);
	CurrentPlayer.setHealth(savedHealth);
	CurrentPlayer.setHeatCap(savedHeatCap);

	std::ifstream savedenemy("savedenemy.txt");															 //open the saved enemy pilot file
	std::getline(savedenemy, pilotName);
	savedenemy >> botName >> savedWeapon1 >> savedWeapon2 >> savedHandling >> savedAim >>	 //read txt file into enemy player object
		savedSpeed >> savedArmor >> savedHealth >> savedHeatCap;
	savedenemy.close();

	CurrentEnemy.setName(pilotName);
	CurrentEnemy.setBotName(botName);
	CurrentEnemy.setWeapon1(savedWeapon1);
	CurrentEnemy.setWeapon2(savedWeapon2);
	CurrentEnemy.setHandling(savedHandling);
	CurrentEnemy.setAim(savedAim);
	CurrentEnemy.setSpeed(savedSpeed);
	CurrentEnemy.setArmor(savedArmor);
	CurrentEnemy.setHealth(savedHealth);
	CurrentEnemy.setHeatCap(savedHeatCap);
	return;																								 //close the enemy pilot file and return to main
}

void returnHowtoplay() {
	std::string outLine;

	std::ifstream howtoplayfile("howtoplay.txt");						//open high score file and output the high score
	if (howtoplayfile.is_open()) {
		while (getline(howtoplayfile, outLine))
			std::cout << outLine << std::endl;
		howtoplayfile.close();											//close the high score file
	}
}

int mainMenuFunction(int Selection) {
	bool repeatMenu;
	char confirm;

	do {
		repeatMenu = false;

		switch (Selection) {
		case 1:
			std::cout << "You have selected NEW GAME, is this correct? (y/n): ";
			std::cin >> confirm;
			if (tolower(confirm) == 'y')
				return 1;
			else {
				std::cout << std::endl << "Well then, please make a new selection: ";
				Selection = selectionChecker();
				repeatMenu = true;
				break;
			}
		case 2:
			std::cout << "You have selected CONTINUE GAME, is this correct? (y/n): ";
			std::cin >> confirm;
			if (tolower(confirm) == 'y')
				return 2;
			else {
				std::cout << std::endl << "Well then, please make a new selection: ";
				Selection = selectionChecker();
				repeatMenu = true;
				break;
			}
		case 3:
			std::cout << "You have selected HOW TO PLAY, is this correct? (y/n): ";
			std::cin >> confirm;
			if (tolower(confirm) == 'y')
				return 3;
			else {
				std::cout << std::endl << "Well then, please make a new selection: ";
				Selection = selectionChecker();
				repeatMenu = true;
				break;
			}
		case 4:
			std::cout << "You have selected EXIT, is this correct? (y/n): ";
			std::cin >> confirm;
			if (tolower(confirm) == 'y')
				return 4;
			else {
				std::cout << std::endl << "Well then, please make a new selection: ";
				Selection = selectionChecker();
				repeatMenu = true;
				break;
			}
		default:
			std::cout << std::endl << "Sorry Dave, I do not understand your input... Please try again: ";
			Selection = selectionChecker();
			repeatMenu = true;
			break;
		}
	} while (repeatMenu);
}

int botSelectFunction(int Selection) {
	bool repeatMenu;
	char confirm;
	std::string outLine;

	std::ifstream botfile1("razor.txt");								//open bot file and output the statistics
	std::ifstream botfile2("maul.txt");
	std::ifstream botfile3("ogre.txt");						

	do {
		repeatMenu = false;
		switch (Selection) {
		case 1:
			std::cout << "You have selected the 'Razor', these are its combat statistics: " << std::endl;
			botfile1.clear();											//seek the bot file to the beginning and read
			botfile1.seekg(botfile1.beg);
			if (botfile1.is_open()) {
				while (getline(botfile1, outLine))						//output bot stats to user
					std::cout << outLine << std::endl;
			}
			std::cout << "Would you like to select this BattleBot? (y/n) : ";				
			std::cin >> confirm;
			if (tolower(confirm) == 'y') {
				botfile1.close();										//close the bot files
				botfile2.close();
				botfile3.close();
				return 1;
			}
			else {
				std::cout << std::endl << "Well then, please make a new selection: ";
				Selection = selectionChecker();
				repeatMenu = true;
				break;
				}
		case 2:
			std::cout << "You have selected the 'Maul', these are its combat statistics: " << std::endl;
			botfile2.clear();											//seek the bot file to the beginning and read
			botfile2.seekg(botfile2.beg);
			if (botfile2.is_open()) {
				while (getline(botfile2, outLine))						//output bot stats to user
					std::cout << outLine << std::endl;
			}
			std::cout << "Would you like to select this BattleBot? (y/n) : ";
			std::cin >> confirm;
			if (tolower(confirm) == 'y') {
				botfile1.close();										//close the bot files
				botfile2.close();
				botfile3.close();
				return 2;
			}
			else {
				std::cout << std::endl << "Well then, please make a new selection: ";
				Selection = selectionChecker();
				repeatMenu = true;
				break;
			}
		case 3:
			std::cout << "You have selected the 'Ogre', these are its combat statistics: " << std::endl;
			botfile3.clear();											//seek the bot file to the beginning and read
			botfile3.seekg(botfile3.beg);						
			if (botfile3.is_open()) {
				while (getline(botfile3, outLine))						//output bot stats to user
					std::cout << outLine << std::endl;
			}
			std::cout << "Would you like to select this BattleBot? (y/n) : ";
			std::cin >> confirm;
			if (tolower(confirm) == 'y') {
				botfile1.close();										//close the bot files
				botfile2.close();
				botfile3.close();
				return 3;
			}
			else {
				std::cout << std::endl << "Well then, please make a new selection: ";
				Selection = selectionChecker();
				repeatMenu = true;
				break;
			}
		default:
			std::cout << std::endl << "Sorry Dave, I do not understand your input... Please try again: ";	//initiate retry loop
			Selection = selectionChecker();
			repeatMenu = true;
			break;
		}
	} while (repeatMenu);		//retry loop
}

int enemybotSelectFunction(int Selection) {
	bool repeatMenu;
	char confirm;

	do {
		repeatMenu = false;

		switch (Selection) {
		case 1:
			std::cout << "You have chosen to battle 'Starwolf' (EASY), are you sure? (y/n): ";
			std::cin >> confirm;
			if (tolower(confirm) == 'y')
				return 1;
			else {
				std::cout << std::endl << "Well then, please make a new selection: ";
				Selection = selectionChecker();
				repeatMenu = true;
				break;
			}
		case 2:
			std::cout << "You have chosen to battle 'Hound' (MEDIUM), are you sure? (y/n): ";
			std::cin >> confirm;
			if (tolower(confirm) == 'y')
				return 2;
			else {
				std::cout << std::endl << "Well then, please make a new selection: ";
				Selection = selectionChecker();
				repeatMenu = true;
				break;
			}
		case 3:
			std::cout << "You have chosen to battle 'Dr.Black' (HARD), are you sure? (y/n): ";
			std::cin >> confirm;
			if (tolower(confirm) == 'y')
				return 3;
			else {
				std::cout << std::endl << "Well then, please make a new selection: ";
				Selection = selectionChecker();
				repeatMenu = true;
				break;
			}
		default:
			std::cout << std::endl << "Sorry Dave, I do not understand your input... Please try again: ";
			Selection = selectionChecker();
			repeatMenu = true;
			break;
		}
	} while (repeatMenu);
}

int locationSelectFunction(int Selection) {
	bool repeatMenu;
	char confirm;

	do {
		repeatMenu = false;

		switch (Selection) {
		case 1:
			std::cout << "You have chosen to fight in the open desert, are you sure? (y/n): ";
			std::cin >> confirm;
			if (tolower(confirm) == 'y')
				return 1;
			else {
				std::cout << std::endl << "Well then, please make a new selection: ";
				Selection = selectionChecker();
				repeatMenu = true;
				break;
			}
		case 2:
			std::cout << "You have chosen to fight in the dense forest, are you sure? (y/n): ";
			std::cin >> confirm;
			if (tolower(confirm) == 'y')
				return 2;
			else {
				std::cout << std::endl << "Well then, please make a new selection: ";
				Selection = selectionChecker();
				repeatMenu = true;
				break;
			}
		case 3:
			std::cout << "You have chosen to fight on the icy ridge, are you sure? (y/n): ";
			std::cin >> confirm;
			if (tolower(confirm) == 'y')
				return 3;
			else {
				std::cout << std::endl << "Well then, please make a new selection: ";
				Selection = selectionChecker();
				repeatMenu = true;
				break;
			}
		default:
			std::cout << std::endl << "Sorry Dave, I do not understand your input... Please try again: ";
			Selection = selectionChecker();
			repeatMenu = true;
			break;
		}
	} while (repeatMenu);
}

int battleFunction(int location, int range, int round) {
	std::string userInput, outLine;
	int toHit, activeHeat, activeRepair, activeDamage; 
	int activeAim = 0, hitMod = 0;	//initialize the aiming bonus to 0, hitMod to 0
	bool battleOver = false;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');												//purge the cin buffer so that getline can be used

	do {
		round++;	//increment the round
		std::cout << std::endl << std::endl << "*******************************ROUND:" << round << "*************************************" << std::endl << std::endl;

		BattleCommand:
		std::cout << std::endl << "<" << CurrentPlayer.getHealth() << " HP> <" << CurrentPlayer.getHeatCap() << " MJ heat to meltdown> <Range: " << range //status display
			<< " dam> <Your enemy has: " << CurrentEnemy.getHealth() << " HP>" << std::endl;
		std::cout << "What will you do? (\"h\" for help): " << std::endl;												//get user command
		std::getline(std::cin, userInput);

		std::cout << std::endl << std::endl << "----------your turn----------" << std::endl << std::endl;				//add a line break to distingush player output from player input

		if (userInput == "h") {
			std::ifstream helpfile("battlehelp.txt");					//open command help file and output to screen
			if (helpfile.is_open()) {
				while (getline(helpfile, outLine))
					std::cout << outLine << std::endl;
				helpfile.close();										//close the command help file
			}
			goto BattleCommand;											//return to the status output and user input command
		}
		else if (userInput == "moveto") {
			range -= (CurrentPlayer.getSpeed() - locations[location].speedPenalty);	//decrement range by playerspeed-speedpenalty
			if (range < 0)	//range cannot go negative, take the absolute value: you ran past the enemy!
				range = abs(range);
			if (locations[location].speedPenalty > 0) {
				std::cout << std::endl << "Fighting in the " << locations[location].locationName << " hinders your movement!" << std::endl;
			}
			std::cout << std::endl << "You slam your control stick forward and run at the enemy!" << std::endl << "You are now: " << range << " dam from " 
				<< CurrentEnemy.getBotName() << "." << std::endl;
		}
		else if (userInput == "moveaway") {
			range += (CurrentPlayer.getSpeed() - locations[location].speedPenalty); //increment range by playerspeed-speedpenalty
			if (locations[location].speedPenalty > 0)
				std::cout << std::endl << "Fighting in the " << locations[location].locationName << " hinders your movement!" << std::endl;
			std::cout << std::endl << "You pull your back on the stick, and your bot backpedals away from the enemy!" 
				<< std::endl << "You are now : " << range << " dam from " << CurrentEnemy.getBotName() << "." << std::endl;
		}
		else if (userInput == "fireprimary") {
			std::cout << "You pull the primary trigger, firing your " << weapons[CurrentPlayer.getWeapon1()].weaponName << " at " << CurrentEnemy.getBotName() << "!" << std::endl;
			activeHeat = (CurrentPlayer.getHeatCap() - weapons[CurrentPlayer.getWeapon1()].weaponHeat);	//drain the reactor's heat capacity
			CurrentPlayer.setHeatCap(activeHeat);
			if (range <= weapons[CurrentPlayer.getWeapon1()].weaponRange) { //check range
				toHit = rand() % 20 + 1;			     //roll 1-20
				hitMod += CurrentPlayer.getAim();	     //increment by the player's aim
				hitMod += CurrentEnemy.getHandling();    //increment by the opponent's handling
				if (activeAim == 5) {				     //check if aim bonus is active
					hitMod += activeAim;				 //increment by the aim bonus
					activeAim = 0;					     //reset the aim bonus parameter
				}
				std::cout << "[Roll: " << toHit << " ]+[Modifier: " << hitMod << " ]     [Target:" << TOHIT_BASE << " ]" << std::endl;
				if ((toHit + hitMod) >= TOHIT_BASE) {
					std::cout << "...and you score a direct hit!" << std::endl;
					activeDamage = (CurrentEnemy.getHealth() - weapons[CurrentPlayer.getWeapon1()].weaponDamage + CurrentEnemy.getArmor());
					CurrentEnemy.setHealth(activeDamage);
					std::cout << CurrentEnemy.getBotName() << "'s armor blocks " << CurrentEnemy.getArmor() << " damage." << std::endl;
					std::cout << CurrentEnemy.getBotName() << " takes " << (weapons[CurrentPlayer.getWeapon1()].weaponDamage-CurrentEnemy.getArmor()) << " damage!" << std::endl;
					hitMod = 0; //reset hitMod to 0 value
				}
				else {
					std::cout << "...but you miss! Your shot couldn't find the target!" << std::endl;
					hitMod = 0; //reset hitMod to 0 value
				}
			}
			else
				std::cout << "...but you are out of range! Your weapon is ineffective!" << std::endl;
		}
		else if (userInput == "firesecondary") {
			std::cout << "You pull the secondary trigger, firing your " << weapons[CurrentPlayer.getWeapon2()].weaponName << " at " << CurrentEnemy.getBotName() << "!" << std::endl;
			activeHeat = (CurrentPlayer.getHeatCap() - weapons[CurrentPlayer.getWeapon2()].weaponHeat);	//drain the reactor's heat capacity
			CurrentPlayer.setHeatCap(activeHeat);
			if (range <= weapons[CurrentPlayer.getWeapon2()].weaponRange) { //check range
				toHit = rand() % 20 + 1;			 //roll 1-20
				hitMod += CurrentPlayer.getAim();	 //increment by the player's aim
				hitMod += CurrentEnemy.getHandling(); //increment by the opponent's handling
				if (activeAim == 5) {				 //check if aim bonus is active
					hitMod += activeAim;				 //increment by the aim bonus
					activeAim = 0;					 //reset the aim bonus parameter
				}
				std::cout << "[Roll: " << toHit << " ]+[Modifier: " << hitMod << " ]     [Target:" << TOHIT_BASE << " ]" << std::endl;
				if ((toHit+hitMod) >= TOHIT_BASE) {
					std::cout << "...and you score a direct hit!" << std::endl;
					activeDamage = (CurrentEnemy.getHealth() - weapons[CurrentPlayer.getWeapon2()].weaponDamage + CurrentEnemy.getArmor());
					CurrentEnemy.setHealth(activeDamage);
					std::cout << CurrentEnemy.getBotName() << "'s armor blocks " << CurrentEnemy.getArmor() << " damage." << std::endl;
					std::cout << CurrentEnemy.getBotName() << " takes " << (weapons[CurrentPlayer.getWeapon2()].weaponDamage - CurrentEnemy.getArmor()) << " damage!" << std::endl;
					hitMod = 0; //reset hitMod to 0 value
				}
				else {
					std::cout << "...but you miss! Your shot couldn't find the target!" << std::endl;
					hitMod = 0; //reset hitMod to 0 value
				}
			}
			else
				std::cout << "...but you are out of range! Your weapon is ineffective!" << std::endl;
		}
		else if (userInput == "aim") {
			std::cout << "You hold your fire, taking careful aim at your opponent for your next shot..." << std::endl;
			activeAim = 5;	//add an aim bonus that will last until the next shot is taken
		}
		else if (userInput == "sinkheat") {
			std::cout << "You slam the big red button marked 'emergency reactor coolant'." << std::endl;
			activeHeat = rand() % 13 + 9; //9-21 heat sunk
			if (activeHeat < 13) {	//output poor result
				std::cout << "You hear the high-pressure sputter of a frozen coolant hose!" <<
					std::endl << "Your reactor sinks: " << activeHeat << " MJ of heat." << std::endl;
			}
			else if (activeHeat < 18) {	//output average result
				std::cout << "You hear a loud 'whoosh' as the coolant mixes with hot reactor water!" <<
					std::endl << "Your reactor sinks: " << activeHeat << " MJ of heat." << std::endl;
			}
			else {						//output best result
				std::cout << "You deftly tune the coolant pressure, pushing a little extra out of the system!" <<
					std::endl << "Your reactor sinks: " << activeHeat << " MJ of heat." << std::endl;
			}
			activeHeat = CurrentPlayer.getHeatCap() + activeHeat - locations[location].heatPenalty;	//redefine activeheat to become the new current heat cap
			CurrentPlayer.setHeatCap(activeHeat);					//store the current heat cap
			if (locations[location].heatPenalty < 0)
				std::cout << "Fighting in the " << locations[location].locationName << " increases the effectiveness of your heat sinks!" << std::endl;
			else if (locations[location].heatPenalty > 0)
				std::cout << "Fighting in the " << locations[location].locationName << " decreases the effectiveness of your heat sinks!" << std::endl;
		}
		else if (userInput == "repair") {
			std::cout << "You open your drone bay and launch your remote controlled repair bots." << std::endl;
			activeRepair = rand() % 10 + 4; //4-13 hp repaired
			if (activeRepair < 7) { //output poor result
				std::cout << "You briefly lose radio contact with your repair bots, yet maintain control!" <<
					std::endl << "You repair and reinforce: " << activeRepair << " HP." << std::endl;
			}
			else if (activeRepair < 11) {
				std::cout << "Your bots whirl around, spraying liquid titanium into weak spots." <<
					std::endl << "You repair and reinforce: " << activeRepair << " HP." << std::endl;
			}
			else {
				std::cout << "Your repair bots buzz quickly as their AI finds a perfect repair algorithm!" <<
					std::endl << "You repair and reinforce: " << activeRepair << " HP." << std::endl;
			}
			activeRepair = CurrentPlayer.getHealth() + activeRepair; //redefine activerepair to become new current health
			CurrentPlayer.setHealth(activeRepair);					 //store the current health level
		}
		else if (userInput == "surrender") {
			std::cout << std::endl << "You signal " << CurrentEnemy.getBotName() << " that you forfeit the match." << std::endl;
			std::cout << "You pull your ejector seat handle--it's all over!" << std::endl;
			CurrentPlayer.setHealth(0);	//set player health to 0
			std::cout << CurrentEnemy.getBotName() << " accepts your surrender." << std::endl;
			goto Surrender;		//skip enemy turn and heat checks and resolve the battle function
		}
		else if (userInput == "stats") {
			std::cout << "Pilot Name: " << CurrentPlayer.getName() << ", piloting the " << CurrentPlayer.getBotName() << " battlebot." << std::endl <<
				"You are " << range << " dam from " << CurrentEnemy.getBotName() << ", your opponent." << std::endl << std::endl <<
				"Your primary weapon is: " << weapons[CurrentPlayer.getWeapon1()].weaponName << std::endl <<
				"Max Range: " << weapons[CurrentPlayer.getWeapon1()].weaponRange << " dam.       Damage: " << weapons[CurrentPlayer.getWeapon1()].weaponDamage <<
				".      " << weapons[CurrentPlayer.getWeapon1()].weaponHeat << " MJ of heat generated." << std::endl <<
				"Your secondary weapon is: " << weapons[CurrentPlayer.getWeapon2()].weaponName << std::endl <<
				"Max Range: " << weapons[CurrentPlayer.getWeapon2()].weaponRange << " dam.       Damage: " << weapons[CurrentPlayer.getWeapon2()].weaponDamage <<
				".      " << weapons[CurrentPlayer.getWeapon2()].weaponHeat << " MJ of heat generated." << std::endl << std::endl <<
				"Your enemies have a " << CurrentPlayer.getHandling() << " to hit modifier.       You have a " << CurrentPlayer.getAim() << " to hit modifier." << std::endl <<
				"Your move speed is: " << CurrentPlayer.getSpeed() << " dam/round.           You have " << CurrentPlayer.getArmor() << " armor." << std::endl << std::endl <<
				"You are fighting in " << locations[location].locationName << "." << std::endl <<
				"which imparts a " << locations[location].heatPenalty << " MJ heat generation modifier, and a " << locations[location].speedPenalty << " dam speed penalty." << std::endl;
			goto BattleCommand;
		}
		else if (userInput == "savegame") {
			std::ofstream outputPlayer("savedpilot.txt");
			outputPlayer << CurrentPlayer.getName() << std::endl << CurrentPlayer.getBotName() << " " << CurrentPlayer.getWeapon1() << " " << CurrentPlayer.getWeapon2() << " " << CurrentPlayer.getHandling() << " " <<
				CurrentPlayer.getAim() << " " << CurrentPlayer.getSpeed() << " " << CurrentPlayer.getArmor() << " " << CurrentPlayer.getHealth() << " " << CurrentPlayer.getHeatCap();
			outputPlayer.close();

			std::ofstream outputEnemy("savedenemy.txt");
			outputEnemy << CurrentEnemy.getName() << std::endl << CurrentEnemy.getBotName() << " " << CurrentEnemy.getWeapon1() << " " << CurrentEnemy.getWeapon2() << " " << CurrentEnemy.getHandling() << " " <<
				CurrentEnemy.getAim() << " " << CurrentEnemy.getSpeed() << " " << CurrentEnemy.getArmor() << " " << CurrentEnemy.getHealth() << " " << CurrentEnemy.getHeatCap();
			outputEnemy.close();

			std::ofstream outputGame("savedgamestate.txt");
			outputGame << location << " " << range << " " << round;
			outputGame.close();

			battleOver = true;
			continue;
		}
		else {
			std::cout << std::endl << "Sorry, I do not understand your input, please try again: ";
			goto BattleCommand;
		}

		if (CurrentPlayer.getHealth() <= 0 || CurrentEnemy.getHealth() <= 0) {	//check to see if anyone is dead after the player turn
			battleOver = true;													//if anyone has <= 0 health, the battle is over!
			continue;															//skip enemy turn and break combat loop
		}
		range = enemyAI(range, location);										//if no one is dead yet, give the enemy a turn, pass the current range and location values

		if (CurrentPlayer.getHeatCap() < 0) {									//check to see if anyone overheated to death after the enemy turn (give advantage to player: if player kills enemy AND overheats, player wins!)
			std::cout << "Alarms shriek and it becomes hard to breathe as hot fumes fill the cockpit!" << std::endl <<
				"Your autoeject system engages as your reactor melts down!" << std::endl;
			CurrentPlayer.setHealth(0);
		}
		else if (CurrentEnemy.getHeatCap() < 0) {
			std::cout << CurrentEnemy.getBotName() << "'s heat vents begin glowing and smoking heavily!" << std::endl <<
				"You see the cockpit glass blow as your enemy ejects from his bot in meltdown!" << std::endl;
			CurrentEnemy.setHealth(0);
		}

		Surrender:	//goto point for surrendering
		if (CurrentPlayer.getHealth() <= 0 || CurrentEnemy.getHealth() <= 0)	//check to see if anyone is dead after the enemy turn
			battleOver = true;													//if anyone has <= 0 health, the battle is over!
		
	} while (!battleOver);

	if (CurrentPlayer.getHealth() <= 0) {										//if the player is dead, return 0
		std::cout << std::endl << "Your battlebot has sustained critical damage! EJECT! EJECT!! EJECT!!!" << std::endl << std::endl << std::endl;
		return 0;
	}
	else if (CurrentEnemy.getHealth() <= 0) {									//if the enemy is dead, return 1
		std::cout << std::endl << "Your opponent's battlebot has sustained critical damage!" << std:: endl << 
			"You see " << CurrentEnemy.getBotName() << " rocket away in their escape pod!" << std::endl << std::endl << std::endl;
		return 1;
	}
	else if (userInput == "savegame") {										    //if the player saved the game, return 2
		std::cout << std::endl << "Game saved successfully!!" << std::endl << std::endl << std::endl;
		return 2;
	}
	else {
		std::cout << std::endl << "Something went wrong!!" << std::endl << std::endl << std::endl;	//error handling -- return 99
		return 99;
	}
}

int enemyAI(int range, int location) {

	int checkRange, checkHeat, checkHealth, randomWeapon, activeRepair, activeHeat, activeDamage, toHit;			//declare local vars
	int hitMod = 0; //initialize hitMod to 0

	checkHealth = CurrentEnemy.getHealth();																			//fill local vars with enemy class parameters and passed values
	checkHeat = CurrentEnemy.getHeatCap();
	checkRange = range;

	std::cout << std::endl << std::endl << "----------enemy turn----------" << std::endl << std::endl;				//add a line break to distingush player output from enemy output

	if (checkHealth < (weapons[CurrentPlayer.getWeapon1()].weaponDamage * 1.1)) {											//always try to repair if health is below 110% the player's primary weapon damage
		std::cout << CurrentEnemy.getBotName() << " opens their drone bay and launchs remote controlled repair bots." << std::endl;
		activeRepair = rand() % 9 + 1;																			    //1-9 hp repaired
		if (activeRepair < 4) {																						//output poor result
			std::cout << CurrentEnemy.getBotName() << " briefly loses radio contact with their repair bots!" << std::endl;
		}
		else if (activeRepair < 8) {																				//output average result
			std::cout << CurrentEnemy.getBotName() << "'s bots whirl around, spraying liquid titanium into weak spots." << std::endl;
		}
		else {																										//output good result
			std::cout << CurrentEnemy.getBotName() << "'s repair bots buzz quickly as their AI finds a perfect repair algorithm!" << std::endl;
		}
		activeRepair = CurrentEnemy.getHealth() + activeRepair;														//redefine activerepair to become new current health
		CurrentEnemy.setHealth(activeRepair);																		//store the current health level for enemy		
	}
	else if (checkHeat < (weapons[CurrentEnemy.getWeapon1()].weaponHeat * 1.4)) {										//else, always sink heat if current heat cap is below  140% primary heat generation
		std::cout << "You see steam pour from " << CurrentEnemy.getBotName() << "'s battle bot." << std::endl;
		activeHeat = rand() % 13 + 9;																				//9-21 heat sunk
		activeHeat = CurrentEnemy.getHeatCap() + activeHeat - locations[location].heatPenalty;						//redefine activeheat to become the new current heat cap
		CurrentEnemy.setHeatCap(activeHeat);																		//store the current heat cap
		if (locations[location].heatPenalty < 0)
			std::cout << "Fighting in the " << locations[location].locationName << " increases the effectiveness of " << CurrentEnemy.getBotName() << "'s heat sinks!" << std::endl;
		else if (locations[location].heatPenalty > 0)
			std::cout << "Fighting in the " << locations[location].locationName << " decreases the effectiveness of " << CurrentEnemy.getBotName() << "'s heat sinks!" << std::endl;
	}
	else if (checkRange > weapons[CurrentEnemy.getWeapon1()].weaponRange) {											//else, always move towards the player if no weapons are in range
		range -= (CurrentEnemy.getSpeed() - locations[location].speedPenalty);										
		if (range < 0)																								//range cannot go negative, take the absolute value: you ran past the enemy!
			range = abs(range);
		if (locations[location].speedPenalty > 0) {
			std::cout << std::endl << "You notice that fighting in the " << locations[location].locationName << " hinders " << CurrentEnemy.getBotName() << "'s movement!" << std::endl;
		}
		std::cout << std::endl << CurrentEnemy.getBotName() << "'s battle bot lumbers thunderously towards you!" << std::endl << "You are now: " << range << " dam from "
			<< CurrentEnemy.getBotName() << "." << std::endl;
	}
	else if (checkRange > weapons[CurrentEnemy.getWeapon2()].weaponRange && checkRange <= weapons[CurrentEnemy.getWeapon1()].weaponRange) { //else, fire primary always if in range of 1 but not 2
		std::cout << CurrentEnemy.getBotName() << " fires their " << weapons[CurrentEnemy.getWeapon1()].weaponName << " at you!" << std::endl;
		activeHeat = (CurrentEnemy.getHeatCap() - weapons[CurrentEnemy.getWeapon1()].weaponHeat);											//drain the reactor's heat capacity
		CurrentEnemy.setHeatCap(activeHeat);
		if (range <= weapons[CurrentEnemy.getWeapon1()].weaponRange) {																		//check range
			toHit = rand() % 20 + 1;																										//roll 1-20
			hitMod += CurrentEnemy.getAim();																									//increment by aim
			hitMod += CurrentPlayer.getHandling();																							//increment by your handling
			std::cout << "[Roll: " << toHit << " ]+[Modifier: " << hitMod << " ]     [Target:" << TOHIT_BASE << " ]" << std::endl;
			if ((toHit+hitMod) >= TOHIT_BASE) {
				std::cout << "...and they score a direct hit!" << std::endl;
				activeDamage = (CurrentPlayer.getHealth() - weapons[CurrentEnemy.getWeapon1()].weaponDamage + CurrentPlayer.getArmor());	//calculate damage to player (HP - damage + armor)
				CurrentPlayer.setHealth(activeDamage);																						//set the player's new health
				std::cout << "Your armor blocks " << CurrentPlayer.getArmor() << " damage." << std::endl;
				std::cout << "Your " << CurrentPlayer.getBotName() << " takes " << (weapons[CurrentEnemy.getWeapon1()].weaponDamage - CurrentPlayer.getArmor()) << " damage!" << std::endl;
				hitMod = 0;	//reset the hit modifier
			}
			else {
				std::cout << "...but they miss!!" << std::endl;
				hitMod = 0;	//reset the hit modifier
			}
		}
		else
			std::cout << "...but they are out of range! Their weapon is ineffective!" << std::endl;
	}
	else {																																		//if none of these priority conditions are met, fire a random weapon!
		randomWeapon = rand() % 100 + 1;																										//roll 1-100

		if (randomWeapon > 50) {																												//fire primary if 51-100
			std::cout << CurrentEnemy.getBotName() << " fires their " << weapons[CurrentEnemy.getWeapon1()].weaponName << " at you!" << std::endl;
			activeHeat = (CurrentEnemy.getHeatCap() - weapons[CurrentEnemy.getWeapon1()].weaponHeat);											//drain the reactor's heat capacity
			CurrentEnemy.setHeatCap(activeHeat);
			if (range <= weapons[CurrentEnemy.getWeapon1()].weaponRange) {																		//check range
				toHit = rand() % 20 + 1;																										//roll 1-20
				hitMod += CurrentEnemy.getAim();																									//increment by aim
				hitMod += CurrentPlayer.getHandling();																							//increment by your handling
				std::cout << "[Roll: " << toHit << " ]+[Modifier: " << hitMod << " ]     [Target:" << TOHIT_BASE << " ]" << std::endl;					//output attack roll
				if ((toHit+hitMod) >= TOHIT_BASE) {
					std::cout << "...and they score a direct hit!" << std::endl;
					activeDamage = (CurrentPlayer.getHealth() - weapons[CurrentEnemy.getWeapon1()].weaponDamage + CurrentPlayer.getArmor());	//calculate damage to player (HP - damage + armor)
					CurrentPlayer.setHealth(activeDamage);	//set the player's new health
					std::cout << "Your armor blocks " << CurrentPlayer.getArmor() << " damage." << std::endl;
					std::cout << "Your " << CurrentPlayer.getBotName() << " takes " << (weapons[CurrentEnemy.getWeapon1()].weaponDamage - CurrentPlayer.getArmor()) << " damage!" << std::endl;
					hitMod = 0;	//reset the hit modifier
				}
				else {
					std::cout << "...but they miss!!" << std::endl;
					hitMod = 0;	//reset the hit modifier
				}
			}
			else
				std::cout << "...but they are out of range! Their weapon is ineffective!" << std::endl;
		}
		else {																																	//fire secondary if 1-50
			std::cout << CurrentEnemy.getBotName() << " fires their " << weapons[CurrentEnemy.getWeapon2()].weaponName << " at you!" << std::endl;
			activeHeat = (CurrentEnemy.getHeatCap() - weapons[CurrentEnemy.getWeapon2()].weaponHeat);											//drain the reactor's heat capacity
			CurrentEnemy.setHeatCap(activeHeat);
			if (range <= weapons[CurrentEnemy.getWeapon2()].weaponRange) {																		//check range
				toHit = rand() % 20 + 1;																										//roll 1-20
				hitMod += CurrentEnemy.getAim();																									//increment by aim
				hitMod += CurrentPlayer.getHandling();																							//increment by your handling
				std::cout << "[Roll: " << toHit << " ]+[Modifier: " << hitMod << " ]     [Target:" << TOHIT_BASE << " ]" << std::endl;					//output attack roll
				if ((toHit+hitMod) >= TOHIT_BASE) {
					std::cout << "...and they score a direct hit!" << std::endl;
					activeDamage = (CurrentPlayer.getHealth() - weapons[CurrentEnemy.getWeapon2()].weaponDamage + CurrentPlayer.getArmor());	//calculate damage to player (HP - damage + armor)
					CurrentPlayer.setHealth(activeDamage);																						//set the player's new health
					std::cout << "Your armor blocks " << CurrentPlayer.getArmor() << " damage." << std::endl;
					std::cout << "Your " << CurrentPlayer.getBotName() << " takes " << (weapons[CurrentEnemy.getWeapon2()].weaponDamage - CurrentPlayer.getArmor()) << " damage!" << std::endl;
					hitMod = 0;	//reset the hit modifier
				}
				else {
					std::cout << "...but they miss!!" << std::endl;
					hitMod = 0;	//reset the hit modifier
				}
			}
			else
				std::cout << "...but they are out of range! Their weapon is ineffective!" << std::endl;
		}
	}

	return range;
}