/**
 * actor.h
 * Represents characters in the game.
 * Contains all actors used in the game.
 * by radj307
 */
#pragma once
#include <string>
#include "Coord.h"
#include "termcolor/termcolor.hpp"

struct ActorMaxStats {
	 int _MAX_HEALTH;
	 int _MAX_STAMINA;
	 int _MAX_DAMAGE;

	ActorMaxStats( unsigned int health, unsigned int stamina, unsigned int damage) : _MAX_HEALTH(health), _MAX_STAMINA(stamina), _MAX_DAMAGE(damage) {}
};

// stats & specs of all actors -- parent struct
struct ActorBase : public ActorMaxStats {
	// Windows console text colors
	enum class color {
		white = 0x0001 | 0x0002 | 0x0004,
		grey = 0,
		yellow = 0x0002 | 0x0004,
		red = 0x0004,
		magenta = 0x0001 | 0x0004,
		blue = 0x0001,
		cyan = 0x0001 | 0x0002,
		green = 0x0002,
	}; color _myColor;

	// convenience boolean that defines this actor as the player
	bool _isPlayer;
	// this actor's attributes
	std::string _name;
	Coord _myPos;
	char _display_char;
	// this actor's current stats
	bool _dead;
	int _health, _stamina;

	ActorBase( std::string myName, Coord& myPos, char myDisplayChar, color myColor, bool isPlayer,  int myMaxHealth,  int myMaxStamina,  int myMaxDamage) : ActorMaxStats(myMaxHealth, myMaxStamina, myMaxDamage), _name(myName), _myPos(myPos), _display_char(myDisplayChar), _myColor(myColor), _health(_MAX_HEALTH), _stamina(_MAX_STAMINA), _isPlayer(isPlayer), _dead(false)  {}
	virtual ~ActorBase() {}

	// decrement y by 1
	void moveU() { _myPos._y--; }
	// increment y by 1
	void moveD() { _myPos._y++; }
	// decrement x by 1
	void moveL() { _myPos._x--; }
	// increment x by 1
	void moveR() { _myPos._x++; }

	// stream insertion operator
	friend inline std::ostream& operator<<(std::ostream &os, ActorBase &a)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<int>(a._myColor)); // set text color to actor's color
		os << a._display_char; // insert actor's character
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 07); // reset text color
		return os;
	}
};

// human player
struct Player : public ActorBase {
	int _discoveryRange;
	checkDistanceFrom _getDist;

	Player( std::string myName, Coord myPos, char myDisplayChar, color myColor,  int discoveryRange = 5) : ActorBase(myName, myPos, myDisplayChar, myColor, true, 100u, 100u, 100u), _discoveryRange(discoveryRange), _getDist(&_myPos) {}
	~Player() {}

	friend inline std::ostream &operator<<(std::ostream &os, Player &p)
	{
		os << p._display_char << ' ';
		return os;
	}
};

// enemy actor
struct Enemy : public ActorBase {
	int _aggro{ 0 };
	Enemy(std::string myName, Coord myPos, char myDisplayChar, color myColor) : ActorBase(myName, myPos, myDisplayChar, myColor, false, 100u, 100u, 20u) {}
	~Enemy() {}
};