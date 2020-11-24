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
	const int _MAX_HEALTH;
	const int _MAX_STAMINA;
	const int _MAX_DAMAGE;

	ActorMaxStats(const int health, const int stamina, const int damage) : _MAX_HEALTH(health), _MAX_STAMINA(stamina), _MAX_DAMAGE(damage) {}
};

// stats & specs of all actors -- parent struct
struct ActorBase : public ActorMaxStats {
	unsigned int _health, _stamina;
	Coord _myPos;
	char _display_char;
	const bool _isPlayer;

	ActorBase(Coord& myPos, char myDisplayChar, bool isPlayer, const int myMaxHealth, const int myMaxStamina, const int myMaxDamage) : ActorMaxStats(myMaxHealth, myMaxStamina, myMaxDamage), _myPos(myPos), _display_char(myDisplayChar), _health(_MAX_HEALTH), _stamina(_MAX_STAMINA), _isPlayer(isPlayer)  {}
	virtual ~ActorBase() {}

	// decrement y by 1
	void moveU()
	{
		_myPos._y--;
	}
	// increment y by 1
	void moveD()
	{
		_myPos._y++;
	}
	// decrement x by 1
	void moveL()
	{
		_myPos._x--;
	}
	// increment x by 1
	void moveR()
	{
		_myPos._x++;
	}

	friend inline std::ostream &operator<<(std::ostream &os, ActorBase &a)
	{
		os << a._display_char << ' ';
		return os;
	}
};

// human player
class Player : public ActorBase {
	

public:
	Player(Coord myPos, char myDisplayChar) : ActorBase(myPos, myDisplayChar, true, 100u, 100u, 100u) {}

	friend inline std::ostream &operator<<(std::ostream &os, Player &p)
	{
		os << termcolor::green << p._display_char << termcolor::reset << ' ';
		return os;
	}
};

// enemy actor
class Enemy : public ActorBase {

public:
	Enemy(Coord myPos, char myDisplayChar) : ActorBase(myPos, myDisplayChar, false, 100u, 100u, 100u) {}
};