/**
 * actor.h
 * Represents characters in the game.
 * Contains all actors used in the game.
 * by radj307
 */
#pragma once
#include <string>
#include <assert.h>
#include "xRand.h"
#include "WinAPI.h"
#include "Coord.h"
#include "termcolor/termcolor.hpp"

const enum class FACTION {
	PLAYER = 0,	// player has faction #0
	ENEMY = 1,	// basic enemy has faction #1
	NEUTRAL = 2,// neutral actor has faction #2
	LAST = 3,
};

// Simple struct containing all of the constant base stats of an actor.
struct ActorMaxStats {
protected:
	int _MAX_HEALTH;
	int _MAX_STAMINA;
	int _MAX_DAMAGE;	// This does not have an associated variable stat.

	/** CONSTRUCTOR **
	 * ActorMaxStats(int, int, int)
	 * 
	 * @param health	- My maximum health value	(Minimum is 10)
	 * @param stamina	- My maximum stamina value	(Minimum is 10)
	 * @param damage	- My damage modifier value	(Minimum is 10)
	 */
	ActorMaxStats(unsigned int Mult, unsigned int HEALTH, unsigned int STAMINA, unsigned int DAMAGE)
	{
		// validate stats
		if ( Mult < 1 )
			Mult = 1;
		// set stats
		_MAX_HEALTH = (signed)(HEALTH * Mult);
		_MAX_STAMINA = (signed)(STAMINA * Mult);
		_MAX_DAMAGE = (signed)(DAMAGE * Mult);
	}

	/** CONSTRUCTOR **
	 * Copy constructor
	 */
	ActorMaxStats(const ActorMaxStats& maxStat) : 
		_MAX_HEALTH(maxStat._MAX_HEALTH), _MAX_STAMINA(maxStat._MAX_STAMINA), _MAX_DAMAGE(maxStat._MAX_DAMAGE) {}

	virtual ~ActorMaxStats() {}

	void refresh_stats(int level)
	{
		_MAX_HEALTH *= level;
		_MAX_STAMINA *= level;
		_MAX_DAMAGE *= level;
	}

public:
	inline int getMaxHealth() { return _MAX_HEALTH; }
	inline int getMaxStamina() { return _MAX_STAMINA; }
	inline int getMaxDamage() { return _MAX_DAMAGE; }
};

// Simple struct containing all of the current values for each stat defined in ActorMaxStats
struct ActorStats : public ActorMaxStats {
protected:
	int _level, _health, _stamina;	// level = Stat Multiplier
	bool _dead;						// Am I dead?
public:
	int _visRange;

	/**
	 * ActorStats(int, int, int)
	 * These values are used to determine my maximum stats.
	 * 
	 * @param health	- My health value
	 * @param stamina	- My stamina value
	 * @param damage	- My damage modifier
	 */
	ActorStats(int level, int health, int stamina, int damage, int visRange) : ActorMaxStats(((level >= 1) ? (level) : (1)), health, stamina, damage), _level((level >= 1) ? (level) : (1)), _health(_MAX_HEALTH), _stamina(_MAX_STAMINA), _dead((_health == 0) ? (true) : (false)), _visRange(visRange) {}

	/** CONSTRUCTOR **
	 * Copy constructor
	 */
	ActorStats(const ActorStats& stat) : ActorMaxStats(stat), _health(stat._health), _stamina(stat._stamina), _level(stat._level), _dead(stat._dead), _visRange(stat._visRange) {}
	~ActorStats() {}

	// Returns this actor's level
	inline int getLevel() { return _level; }
	inline void addLevel() 
	{ 
		_level++;
		refresh_stats(_level);
	}
	inline void subLevel() { ((_level > 1) ? (_level--) : (_level = 1)); }
	/**
	 * getHealth()
	 * Returns a copy of this actor's health value
	 *
	 * @returns int
	 */
	inline int getHealth() { return _health; }
	/**
	 * setHealth(int)
	 * Sets the actor's health to a new value, and automatically sets the dead flag if it is below 0.
	 *
	 * @param newValue	- The new health value
	 * @returns int		- The new health value
	 */
	inline int setHealth(int newValue)
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
	inline int modHealth(int modValue)
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
	inline int getStamina() { return _stamina; }
	/**
	 * setStamina(int)
	 * Sets the actor's stamina to a new value.
	 *
	 * @param newValue	- The new stamina value
	 * @returns int		- The new stamina value
	 */
	inline int setStamina(int newValue)
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
	inline int modStamina(int modValue)
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
};

struct ActorTemplate {
	ActorStats _stats;
	char _char;
	WinAPI::color _color;

	ActorTemplate(ActorStats templateStats, char templateChar, WinAPI::color templateColor) : _stats(templateStats), _char(templateChar), _color(templateColor) {}
};

// stats & specs of all actors -- parent struct
struct ActorBase : public ActorStats {
protected:
	std::string _name;		// My reporting name
	FACTION _faction;		// My faction
	Coord _pos;				// My position as a coordinate
	char _char;				// My displayed char
	WinAPI::color _color;	// My displayed char's color in the console
	std::vector<FACTION> _hostileTo;
	int _kill_count;

private:
	// Initialize this actor's hostile faction list -- Sets all factions as hostile
	inline void initHostilities()
	{
		for ( int i = static_cast<int>(FACTION::PLAYER); i < static_cast<int>(FACTION::LAST); i++ ) {
			FACTION thisFaction{ static_cast<FACTION>(i) };
			if ( thisFaction != _faction )
				_hostileTo.push_back(thisFaction);
		}
	}

public:
	checkDistanceFrom getDist;		// Functor that gets the distance from _pos to a given Coord point

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
	ActorBase(FACTION myFaction, std::string myName, Coord myPos, char myChar, WinAPI::color myColor, ActorStats myStats) : ActorStats(myStats), _faction(myFaction), _name(myName), _pos(myPos), _char(myChar), _color(myColor), _kill_count(0), getDist(&_pos)
	{
		initHostilities();
	}
	ActorBase(FACTION myFaction, std::string myName, Coord myPos, ActorTemplate& myTemplate) : ActorStats(myTemplate._stats), _faction(myFaction), _name(myName), _pos(myPos), _char(myTemplate._char), _color(myTemplate._color), _kill_count(0), getDist(&_pos)
	{
		initHostilities();
	}
	/** DESTRUCTOR **
	 */
	virtual ~ActorBase() {}

	// Movement functions, these do not check if a movement was possible, they are simply a wrapper for incrementing/decrementing _pos

	inline void moveU() { _pos._y--; } // decrement y by 1
	inline void moveD() { _pos._y++; } // increment y by 1
	inline void moveL() { _pos._x--; } // decrement x by 1
	inline void moveR() { _pos._x++; } // increment x by 1

	// Set this actor's relationship to another faction
	inline void setRelationship(FACTION faction, bool hostile)
	{
		for ( size_t it{ 0 }; it < _hostileTo.size(); it++ ) {
			if ( _hostileTo.at(it) == faction ) {
				if ( !hostile ) // remove faction hostility
					_hostileTo.erase(_hostileTo.begin() + it);
				else return; // faction is already hostile
			}
		}
		// Add this faction as hostile
		_hostileTo.push_back(faction);
	}

	// returns true if given faction is hostile
	inline bool isHostileTo(FACTION target)
	{
		for ( auto it : _hostileTo )
			if ( it == target )
				return true;
		return false;
	}
	
	inline std::string name() { return _name; }
	inline FACTION faction() { return _faction; }
	inline Coord pos() { return _pos; }
	inline bool isDead() { return _dead; }
	inline int getVis() { return _visRange; }
	inline char getChar() { return _char; }
	inline int getKills() { return _kill_count; }
	inline void addKill() { _kill_count++; }


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
	Player(std::string myName, Coord myPos, char myChar, WinAPI::color myColor, ActorStats myStats) :
		ActorBase(FACTION::PLAYER, myName, myPos, myChar, myColor, myStats) {}
	Player(std::string myName, Coord myPos, ActorTemplate& myTemplate) :
		ActorBase(FACTION::PLAYER, myName, myPos, myTemplate) {}
	/** DESTRUCTOR **
	 */
	~Player() {}
};

// base NPC
struct NPC : public ActorBase {
protected:
	int _aggro;	// Aggression value used to determine how long this enemy will follow the player before giving up.
	ActorBase* _target;
public:

	NPC(FACTION myFaction, std::string myName, Coord myPos, char myChar, WinAPI::color myColor, ActorStats myStats) : ActorBase(myFaction, myName, myPos, myChar, myColor, myStats), _aggro(0), _target(nullptr) {}
	NPC(FACTION myFaction, std::string myName, Coord myPos, ActorTemplate& myTemplate) : ActorBase(myFaction, myName, myPos, myTemplate), _aggro(0), _target(nullptr) {}
	virtual ~NPC() {}

	// Returns if this enemy is currently aggravated
	inline bool isAggro()
	{
		if ( _aggro == 0 )
			return false;
		return true;
	}
	// Returns this enemy's aggression value
	inline int getAggro() { return _aggro; }
	// Modify this enemy's aggression, positive adds, negative removes.
	inline void modAggro(int modValue)
	{
		int newValue{ _aggro + modValue };
		if ( newValue < 0 )
			newValue = 0;
		_aggro = newValue;
	}
	// Decreases this enemy's aggression by 1
	inline void decrementAggro()
	{
		if ( (_aggro - 1) <= 0 )
			_aggro = 0;
		else _aggro--;
	}

	inline ActorBase* getTarget() { return _target; }
	inline void setTarget(ActorBase* target) 
	{ 
		if ( !isHostileTo(target->faction()) )
			setRelationship(target->faction(), true);
		_target = target;
	}
	inline void removeTarget() { _target = nullptr; }
};

// enemy actor
struct Enemy : public NPC {
//	Enemy() : NPC(FACTION::ENEMY, "Enemy", Coord(-1, -1), 0, '~', WinAPI::color::grey, 80, 50, 20) {}
	/** CONSTRUCTOR **
	 * Enemy(string, Coord, char, WinAPI::color)
	 * Constructs a basic enemy with the given parameters.
	 *
	 * @param myName	- My reporting name.
	 * @param myPos		- My current position as a matrix coordinate
	 * @param myChar	- My display character when inserted into a stream
	 * @param myColor	- My character's color when inserted into a stream
	 */
	Enemy(std::string myName, Coord myPos, char myChar, WinAPI::color myColor, int myLevel, int myHealth, int myStamina, int myDamage, int myVisRange) : NPC(FACTION::ENEMY, myName, myPos, myChar, myColor, ActorStats(myLevel, myHealth, myStamina, myDamage, myVisRange)) {}
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
	Enemy(std::string myName, Coord myPos, char myChar, WinAPI::color myColor, ActorStats myStats) : NPC(FACTION::ENEMY, myName, myPos, myChar, myColor, myStats) {}
	Enemy(std::string myName, Coord myPos, ActorTemplate& myTemplate) : NPC(FACTION::ENEMY, myName, myPos, myTemplate) {}
	~Enemy() {}
};

// neutral actor
struct Neutral : public NPC {
//	Neutral() : NPC(FACTION::NEUTRAL, "Neutral Actor", Coord(-1, -1), 0, '~', WinAPI::color::grey, 80, 50, 20) {}
	Neutral(std::string myName, Coord myPos, char myChar, WinAPI::color myColor, int myLevel, int myHealth, int myStamina, int myDamage, int myVisRange) : NPC(FACTION::NEUTRAL, myName, myPos, myChar, myColor, ActorStats(myLevel, myHealth, myStamina, myDamage, myVisRange)) {}
	Neutral(std::string myName, Coord myPos, char myChar, WinAPI::color myColor, ActorStats myStats) : NPC(FACTION::NEUTRAL, myName, myPos, myChar, myColor, myStats) {}
	Neutral(std::string myName, Coord myPos, ActorTemplate& myTemplate) : NPC(FACTION::NEUTRAL, myName, myPos, myTemplate) {}
	~Neutral() {}
};