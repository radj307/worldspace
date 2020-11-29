/**
 * actor.h
 * Represents characters in the game.
 * Contains all actors used in the game.
 * by radj307
 */
#pragma once
#include <string>
#include "xRand.h"
#include "WinAPI.h"
#include "Coord.h"
#include "termcolor/termcolor.hpp"

const enum class FACTION {
	PLAYER = 0,	// player has faction #0
	ENEMY = 1,	// basic enemy has faction #1
	NEUTRAL = 2,// neutral actor has faction #2
};

// Simple struct containing all of the constant base stats of an actor.
struct ActorMaxStats {
	int _MAX_HEALTH;
	int _MAX_STAMINA;
	int _MAX_DAMAGE;	// This does not have an associated variable stat.

protected:
	/** CONSTRUCTOR **
	 * ActorMaxStats(int, int, int)
	 * 
	 * @param health	- My maximum health value	(Minimum is 10)
	 * @param stamina	- My maximum stamina value	(Minimum is 10)
	 * @param damage	- My damage modifier value	(Minimum is 10)
	 */
	ActorMaxStats(const int HEALTH, const int STAMINA, const int DAMAGE) : 
		_MAX_HEALTH(HEALTH), _MAX_STAMINA(STAMINA), _MAX_DAMAGE(DAMAGE) 
	{
		if ( _MAX_HEALTH < 10 )
			_MAX_HEALTH = 10;
		if ( _MAX_STAMINA < 10 )
			_MAX_STAMINA = 10;
		if ( _MAX_DAMAGE < 10 )
			_MAX_DAMAGE = 10;
	}
	/** CONSTRUCTOR **
	 * Copy constructor
	 */
	ActorMaxStats(const ActorMaxStats& maxStat) : 
		_MAX_HEALTH(maxStat._MAX_HEALTH), _MAX_STAMINA(maxStat._MAX_STAMINA), _MAX_DAMAGE(maxStat._MAX_DAMAGE) {}
};

// Simple struct containing all of the current values for each stat defined in ActorMaxStats
struct ActorStats : public ActorMaxStats {
protected:
	int _health, _stamina;
	
	/**
	 * ActorStats(int, int, int)
	 * These values are used to determine my maximum stats.
	 * 
	 * @param health	- My health value
	 * @param stamina	- My stamina value
	 * @param damage	- My damage modifier
	 */
	ActorStats(int health, int stamina, int damage) : ActorMaxStats(health, stamina, damage), _health(health), _stamina(stamina) {}
	/** CONSTRUCTOR **
	 * Copy constructor
	 */
	ActorStats(const ActorStats& stat) : ActorMaxStats(stat), _health(stat._health), _stamina(stat._stamina) {}
};

// stats & specs of all actors -- parent struct
struct ActorBase : public ActorStats {
public:
	std::string _name;		// My reporting name
	FACTION _faction;		// My faction
	Coord _pos;				// My position as a coordinate
	char _char;				// My displayed char
	WinAPI::color _color;	// My displayed char's color in the console
	bool _dead;				// Am I dead?

	/** CONSTRUCTOR **
	 * ActorBase(FACTION, string, Coord, char, WinAPI::color, int, int, int)
	 * This is the base constructor for actor types.
	 * 
	 * @param myFaction	- My faction / group of actors.
	 * @param myName	- My reporting name.
	 * @param myPos		- My current position as a matrix coordinate
	 * @param myChar	- My display character when inserted into a stream
	 * @param myColor	- My character's color when inserted into a stream
	 * @param myStats	- My base statistics
	 */
	ActorBase(FACTION myFaction, std::string myName, Coord myPos, char myChar, WinAPI::color myColor, ActorStats& myStats) : ActorStats(myStats), _faction(myFaction), _name(myName), _pos(myPos), _char(myChar), _color(myColor), _dead(false)  {}
	/** CONSTRUCTOR **
	 * ActorBase(FACTION, string, Coord, char, WinAPI::color, int, int, int)
	 * This is the base constructor for actor types.
	 *
	 * @param myFaction	- My faction / group of actors.
	 * @param myName	- My reporting name.
	 * @param myPos		- My current position as a matrix coordinate
	 * @param myChar	- My display character when inserted into a stream
	 * @param myColor	- My character's color when inserted into a stream
	 * @param myHealth	- My (max) health value
	 * @param myStamina	- My (max) stamina value
	 * @param myDamage	- My (max) damage modifier
	 */
	ActorBase(FACTION myFaction, std::string myName, Coord myPos, char myChar, WinAPI::color myColor, int myHealth, int myStamina, int myDamage) : ActorStats(myHealth, myStamina, myDamage), _faction(myFaction), _name(myName), _pos(myPos), _char(myChar), _color(myColor), _dead(false)  {}
	/** DESTRUCTOR **
	 */
	virtual ~ActorBase() {}

	// Movement functions, these do not check if a movement was possible, they are simply a wrapper for incrementing/decrementing _pos

	void moveU() { _pos._y--; } // decrement y by 1
	void moveD() { _pos._y++; } // increment y by 1
	void moveL() { _pos._x--; } // decrement x by 1
	void moveR() { _pos._x++; } // increment x by 1

	/**
	 * getHealth()
	 * Returns a copy of this actor's health value
	 *
	 * @returns int
	 */
	int getHealth() { return _health; }
	/**
	 * setHealth(int)
	 * Sets the actor's health to a new value, and automatically sets the dead flag if it is below 0.
	 * 
	 * @param newValue	- The new health value
	 * @returns int		- The new health value
	 */
	int setHealth(int newValue)
	{
		// check if the new value is above the max
		if ( newValue > _MAX_HEALTH )
			newValue = _MAX_HEALTH;
		// check if the new value is below or equal to 0
		else if ( newValue <= 0 ) {
			newValue = 0;
			_dead = true;
		}
		// set the new value
		_health = newValue;
		return _health;
	}
	/**
	 * modHealth(int)
	 * Modifies the actor's health value, and automatically sets the dead flag if it drops below 0.
	 * 
	 * @param modValue	- The amount to modify health by, negative removes, positive adds.
	 * @returns int		- The new health value
	 */
	int modHealth(int modValue)
	{
		int newHealth{ _health + modValue };
		// If the new value of health is above the max, set it to 0
		if ( newHealth > _MAX_HEALTH )
			_health = _MAX_HEALTH;
		// If the new value of health is equal or below 0, set it to 0
		else if ( newHealth <= 0 ) {
			_health = 0;
			_dead = true;
		}
		// Else, set health to newHealth.
		else _health = newHealth;
		return _health;
	}
	/**
	 * getStamina()
	 * Returns a copy of this actor's stamina value
	 *
	 * @returns int
	 */
	int getStamina() { return _stamina; }
	/**
	 * setStamina(int)
	 * Sets the actor's stamina to a new value.
	 * 
	 * @param newValue	- The new stamina value
	 * @returns int		- The new stamina value
	 */
	int setStamina(int newValue)
	{
		// check if the new value is above the max
		if ( newValue > _MAX_STAMINA )
			newValue = _MAX_STAMINA;
		// check if the new value is below or equal to 0
		else if ( newValue <= 0 )
			newValue = 0;
		// set the new value
		_stamina = newValue;
		return _stamina;
	}
	/**
	 * modStamina(int)
	 * Modifies the actor's stamina value.
	 * 
	 * @param modValue	- The amount to modify stamina by, negative removes, positive adds.
	 * @returns int		- The new stamina value
	 */
	int modStamina(int modValue)
	{
		int newStamina{ _stamina + modValue };
		// If the new value of stamina is above the max, set it to 0
		if ( newStamina > _MAX_STAMINA )
			_stamina = _MAX_STAMINA;
		// If the new value of stamina is equal or below 0, set it to 0
		else if ( newStamina <= 0 )
			_stamina = 0;
		// Else, set stamina to newStamina.
		else _stamina = newStamina;
		return _stamina;
	}

	// stream insertion operator
	friend inline std::ostream& operator<<(std::ostream &os, ActorBase &a)
	{
		// set text color to actor's color
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<int>(a._color));

		// insert actor's character
		os << a._char;

		// reset text color
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 07);
		return os;
	}
};

// human player
struct Player : public ActorBase {
	int _discoveryRange;			// This is the radius of tile discovery around the player's position
	checkDistanceFrom getDist;		// Functor that gets the distance from pos to a given Coord point

	/** CONSTRUCTOR **
	 * Player(FACTION, string, Coord, char, WinAPI::color, int)
	 * This is the base constructor for actor types.
	 *
	 * @param myName	- My reporting name.
	 * @param myPos		- My current position as a matrix coordinate
	 * @param myChar	- My display character when inserted into a stream
	 * @param myColor	- My character's color when inserted into a stream
	 * @param visRange	- (Default: 5) My sight range / tile discovery radius
	 */
	Player(std::string myName, Coord myPos, char myChar, WinAPI::color myColor,  int visRange = 5) :
		ActorBase(FACTION::PLAYER, myName, myPos, myChar, myColor, 100, 100, 35), _discoveryRange(visRange), getDist(&_pos) {}
	/** CONSTRUCTOR **
	 * Player(FACTION, string, Coord, char, WinAPI::color, int, ActorStats&)
	 * This is the base constructor for actor types.
	 *
	 * @param myName	- My reporting name.
	 * @param myPos		- My current position as a matrix coordinate
	 * @param myChar	- My display character when inserted into a stream
	 * @param myColor	- My character's color when inserted into a stream
	 * @param visRange	- My sight range / tile discovery radius
	 * @param myStats	- A ref to my base statistics object
	 */
	Player(std::string myName, Coord myPos, char myChar, WinAPI::color myColor,  int visRange, ActorStats& myStats) :
		ActorBase(FACTION::PLAYER, myName, myPos, myChar, myColor, myStats), _discoveryRange(visRange), getDist(&_pos) {}
	/** DESTRUCTOR **
	 */
	~Player() {}
};

// enemy actor
struct Enemy : public ActorBase {
protected:
	int _aggro{ 0 };	// Aggression value used to determine how long this enemy will follow the player before giving up.
public:

	Enemy() : ActorBase(FACTION::ENEMY, "Enemy", Coord(-1, -1), '~', WinAPI::color::grey, 80, 50, 20) {}
	/** CONSTRUCTOR **
	 * Enemy(string, Coord, char, WinAPI::color)
	 * Constructs a basic enemy with the given parameters.
	 *
	 * @param myName	- My reporting name.
	 * @param myPos		- My current position as a matrix coordinate
	 * @param myChar	- My display character when inserted into a stream
	 * @param myColor	- My character's color when inserted into a stream
	 */
	Enemy(std::string myName, Coord myPos, char myChar, WinAPI::color myColor, FACTION myFaction = FACTION::ENEMY) : ActorBase(myFaction, myName, myPos, myChar, myColor, 80, 50, 20) {}
	/** CONSTRUCTOR **
	 * Enemy(string, Coord, char, WinAPI::color, ActorStats&)
	 * Constructs a basic enemy with the given parameters.
	 *
	 * @param myName	- My reporting name.
	 * @param myPos		- My current position as a matrix coordinate
	 * @param myChar	- My display character when inserted into a stream
	 * @param myColor	- My character's color when inserted into a stream
	 * @param myStats	- A ref to my base statistics object
	 */
	Enemy(std::string myName, Coord myPos, char myChar, WinAPI::color myColor, ActorStats& myStats, FACTION myFaction = FACTION::ENEMY) : ActorBase(myFaction, myName, myPos, myChar, myColor, myStats) {}
	/** DESTRUCTOR **
	 */
	~Enemy() {}

	// Returns if this enemy is currently aggravated
	bool isAggro()
	{
		if ( _aggro == 0 )
			return false;
		return true;
	}
	// Returns this enemy's aggression value
	int getAggro() { return _aggro; }
	// Modify this enemy's aggression, positive adds, negative removes.
	void modAggro(int modValue)
	{
		int newValue{ _aggro + modValue };
		if ( newValue < 0 )
			newValue = 0;
		_aggro = newValue;
	}
	// Decreases this enemy's aggression by 1
	void decrementAggro()
	{
		if ( (_aggro - 1) <= 0 )
			_aggro = 0;
		else _aggro--;
	}

	// Returns a randomly generated enemy
	static inline Enemy rng_build(_xRand_internal::xRand* rngEngine, Coord pos, WinAPI::color myColor = WinAPI::color::red, FACTION faction = FACTION::ENEMY, std::string name = "Enemy")
	{
		if ( rngEngine != nullptr ) {
			return{ name, pos, char(rngEngine->get(96, 64)), myColor, faction };
		}
		else return{ Enemy() };
	}
	// Returns a randomly generated enemy
	static inline Enemy rng_build(_xRand_internal::xRand* rngEngine, Coord pos, char myChar, WinAPI::color myColor = WinAPI::color::red, FACTION faction = FACTION::ENEMY, std::string name = "Enemy")
	{
		if ( rngEngine != nullptr ) {
			return{ name, pos, myChar, myColor, faction };
		}
		else return{ Enemy() };
	}
	// Returns a randomly generated enemy
	static inline Enemy rng_build(_xRand_internal::xRand* rngEngine, ActorStats myStats, Coord pos, char myChar, WinAPI::color myColor = WinAPI::color::red, FACTION faction = FACTION::ENEMY, std::string name = "Enemy")
	{
		if ( rngEngine != nullptr ) {
			return{ name, pos, myChar, myColor, myStats, faction };
		}
		else return{ Enemy() };
	}
};

// neutral actor
struct Neutral : public ActorBase {
protected:
	int _aggro{ 0 };			// Aggression value used to determine how long this enemy will follow the player before giving up.
public:
	checkDistanceFrom getDist;	// Functor that gets the distance from pos to a given Coord point
	bool _isHostileToPlayer;
	bool _isHostileToEnemy;

	Neutral() : ActorBase(FACTION::NEUTRAL, "Neutral Actor", Coord(-1, -1), '~', WinAPI::color::grey, 80, 50, 20), getDist(&_pos), _isHostileToPlayer(false), _isHostileToEnemy(false) {}
	Neutral(std::string myName, Coord myPos, char myChar, WinAPI::color myColor, FACTION myFaction = FACTION::NEUTRAL) : ActorBase(myFaction, myName, myPos, myChar, myColor, 80, 50, 20), getDist(&_pos), _isHostileToPlayer(false), _isHostileToEnemy(false) {}
	Neutral(std::string myName, Coord myPos, char myChar, WinAPI::color myColor, ActorStats& myStats, FACTION myFaction = FACTION::NEUTRAL) : ActorBase(myFaction, myName, myPos, myChar, myColor, myStats), getDist(&_pos), _isHostileToPlayer(false), _isHostileToEnemy(false) {}

	// Returns if this actor is currently aggravated
	bool isAggro()
	{
		if ( _aggro == 0 )
			return false;
		return true;
	}
	// Returns this actor's aggression value
	int getAggro() { return _aggro; }
	// Modify this actor's aggression, positive adds, negative removes.
	void modAggro(int modValue)
	{
		int newValue{ _aggro + modValue };
		if ( newValue < 0 )
			newValue = 0;
		_aggro = newValue;
	}
	// Decreases this actor's aggression by 1
	void decrementAggro()
	{
		if ( (_aggro - 1) <= 0 )
			_aggro = 0;
		else _aggro--;
	}

	// Returns a randomly generated Neutral
	static inline Neutral rng_build(_xRand_internal::xRand* rngEngine, Coord pos, WinAPI::color myColor = WinAPI::color::cyan, FACTION faction = FACTION::NEUTRAL, std::string name = "Neutral Actor")
	{
		if ( rngEngine != nullptr ) {
			return{ name, pos, char(rngEngine->get(96, 64)), myColor, faction };
		}
		else return{ Neutral() };
	}
	// Returns a randomly generated Neutral
	static inline Neutral rng_build(_xRand_internal::xRand* rngEngine, Coord pos, char myChar, WinAPI::color myColor = WinAPI::color::cyan, FACTION faction = FACTION::NEUTRAL, std::string name = "Neutral Actor")
	{
		if ( rngEngine != nullptr ) {
			return{ name, pos, myChar, myColor, faction };
		}
		else return{ Neutral() };
	}
	// Returns a randomly generated Neutral
	static inline Neutral rng_build(_xRand_internal::xRand* rngEngine, ActorStats myStats, Coord pos, char myChar, WinAPI::color myColor = WinAPI::color::cyan, FACTION faction = FACTION::NEUTRAL, std::string name = "Neutral Actor")
	{
		if ( rngEngine != nullptr ) {
			return{ name, pos, myChar, myColor, myStats, faction };
		}
		else return{ Neutral() };
	}
};