/**
 * actor.h
 * Represents characters in the game.
 * Contains all actors used in the game.
 * by radj307
 */
#pragma once
#include <string>
#include <utility>
#include <vector>
#include "Coord.h"
#include "WinAPI.h"
#include "termcolor/termcolor.hpp"

enum class FACTION {
	PLAYER = 0,	// player has faction #0
	ENEMY = 1,	// basic enemy has faction #1
	NEUTRAL = 2,// neutral actor has faction #2
	NONE = 3,
};

// Simple struct containing all of the constant base stats of an actor.
struct ActorMaxStats {
protected:
	// These are modified every level change
	int _MAX_HEALTH, _MAX_STAMINA, _MAX_DAMAGE;
	// Do not modify these
	int _BASE_HEALTH, _BASE_STAMINA, _BASE_DAMAGE;

	/** CONSTRUCTOR **
	 * ActorMaxStats(int, int, int)
	 *
	 * @param Mult		- My level, aka stat mult.	(Minimum is 1)
	 * @param HEALTH	- My maximum health value	(Minimum is 10)
	 * @param STAMINA	- My maximum stamina value	(Minimum is 10)
	 * @param DAMAGE	- My damage modifier value	(Minimum is 10)
	 */
	ActorMaxStats(const unsigned int Mult, const unsigned int HEALTH, const unsigned int STAMINA, const unsigned int DAMAGE) : _MAX_HEALTH(static_cast<signed>(HEALTH * (Mult < 1 ? 1 : Mult))), _MAX_STAMINA(static_cast<signed>(STAMINA * (Mult < 1 ? 1 : Mult))), _MAX_DAMAGE(static_cast<signed>(DAMAGE * (Mult < 1 ? 1 : Mult))), _BASE_HEALTH(_MAX_HEALTH), _BASE_STAMINA(_MAX_STAMINA), _BASE_DAMAGE(_MAX_DAMAGE) {}
	ActorMaxStats(const ActorMaxStats&) = default;
	ActorMaxStats(ActorMaxStats&&) = default;
	virtual ~ActorMaxStats() = default;
	ActorMaxStats& operator=(const ActorMaxStats&) = default;
	ActorMaxStats& operator=(ActorMaxStats&&) = default;
	
public:
	[[nodiscard]] int getMaxHealth() const { return _MAX_HEALTH; }
	[[nodiscard]] int getMaxStamina() const { return _MAX_STAMINA; }
	[[nodiscard]] int getMaxDamage() const { return _MAX_DAMAGE; }
};

// Simple struct containing all of the current values for each stat defined in ActorMaxStats
struct ActorStats : ActorMaxStats {
protected:
	int _level, _health, _stamina;	// level = Stat Multiplier
	bool _dead;						// Am I dead?
	int _visRange;					// Range in tiles that this actor can see

	/**
	 * update_stats(int)
	 * Sets this actor's level to a new value, and updates their stats accordingly. This function is called by addLevel()
	 *
	 * @param newLevel	- The new level to set
	 */
	void update_stats(const int newLevel)
	{
		_level = newLevel;
		if ( _level % 3 == 0 ) {
			_MAX_HEALTH = static_cast<int>(static_cast<float>(_BASE_HEALTH) * (static_cast<float>(_level) / 1.5f));
			_MAX_STAMINA = static_cast<int>(static_cast<float>(_BASE_STAMINA) * (static_cast<float>(_level) / 1.5f));
			_MAX_DAMAGE = static_cast<int>(static_cast<float>(_BASE_DAMAGE) * (static_cast<float>(_level) / 1.5f));
		}
	}

	void restore_all_stats()
	{
		_health = _MAX_HEALTH;
		_stamina = _MAX_STAMINA;
	}
	
public:
	/**
	 * ActorStats(int, int, int)
	 * These values are used to determine my maximum stats.
	 * 
	 * @param level		- My level
	 * @param health	- My health value
	 * @param stamina	- My stamina value
	 * @param damage	- My damage modifier
	 * @param visRange	- My sight range
	 */
	ActorStats(const int level, const int health, const int stamina, const int damage, const int visRange) : ActorMaxStats(level >= 1 ? level : 1, health, stamina, damage), _level(level >= 1 ? level : 1), _health(_MAX_HEALTH), _stamina(_MAX_STAMINA), _dead(_health == 0 ? true : false), _visRange(visRange) {}

	// This function will set both the maximum & base values for a given stat, as well as restoring all stats to max.
	void setMaxHealth(const unsigned int newValue)
	{
		_MAX_HEALTH = static_cast<signed>(newValue);
		_BASE_HEALTH = _MAX_HEALTH;
		restore_all_stats();
	}
	// This function will set both the maximum & base values for a given stat, as well as restoring all stats to max.
	void setMaxStamina(const unsigned int newValue)
	{
		_MAX_STAMINA = static_cast<signed>(newValue);
		_BASE_STAMINA = _MAX_STAMINA;
		restore_all_stats();
	}
	// This function will set both the maximum & base values for a given stat, as well as restoring all stats to max.
	void setMaxDamage(const unsigned int newValue)
	{
		_MAX_DAMAGE = static_cast<signed>(newValue);
		_BASE_DAMAGE = _MAX_DAMAGE;
		restore_all_stats();
	}

	// Returns this actor's level
	[[nodiscard]] int getLevel() const { return _level; }
	// Get this actor's visibility range
	[[nodiscard]] auto getVis() const -> int { return _visRange; }
	// Increases this actor's level by one
	void addLevel() 
	{ 
		update_stats(++_level);
	}
	// Decreases this actor's level by one
	void subLevel()
	{
		update_stats(_level > 1 ? --_level : _level = 1);
	}
	/**
	 * getHealth()
	 * Returns a copy of this actor's health value
	 *
	 * @returns int
	 */
	[[nodiscard]] int getHealth() const { return _health; }
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
	int modHealth(const int modValue)
	{
		const auto newHealth{ _health + modValue };
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
	[[nodiscard]] int getStamina() const { return _stamina; }
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
	int modStamina(const int modValue)
	{
		const auto newStamina{ _stamina + modValue };
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

// Used to create stat templates for generating large quantities of actors at once
struct ActorTemplate {
	std::string _name;
	ActorStats _stats;
	char _char;
	WinAPI::color _color;
	std::vector<FACTION> _hostile_to;
	int _max_aggression;
	unsigned int _chance;

	/**
	 * ActorTemplate(string, ActorStats&, char, WinAPI::color, vector<FACTION>)
	 * @brief Constructs a human player actor template.
	 *
	 * @param templateName	- A name of type string.
	 * @param templateStats	- Ref to an ActorStats instance. (level, health, stamina, damage, visible range)
	 * @param templateChar	- A character to represent actors of this type
	 * @param templateColor	- A color to represent actors of this type
	 */
	ActorTemplate(std::string templateName, ActorStats templateStats, const char templateChar, const WinAPI::color templateColor) : _name(std::move(templateName)), _stats(std::move(templateStats)), _char(templateChar), _color(templateColor), _max_aggression(0), _chance(100u) {}
	/**
	 * ActorTemplate(string, ActorStats&, char, WinAPI::color, vector<FACTION>)
	 * @brief Constructs a NPC actor template. This template type is used to randomly generate NPCs
	 *
	 * @param templateName	- A name of type string.
	 * @param templateStats	- Ref to an ActorStats instance. (level, health, stamina, damage, visible range)
	 * @param templateChar	- A character to represent actors of this type
	 * @param templateColor	- A color to represent actors of this type
	 * @param hostileTo		- A list of factions that actors of this type are hostile to
	 * @param spawnChance	- The chance that this actor template will be chosen over another. valid: (0-100)
	 * @param maxAggro		- The number of move cycles passed before this actor loses its target.
	 */
	ActorTemplate(std::string templateName, ActorStats templateStats, const char templateChar, const WinAPI::color templateColor, std::vector<FACTION> hostileTo, const int maxAggro, const unsigned int spawnChance) : _name(std::move(templateName)), _stats(std::move(templateStats)), _char(templateChar), _color(templateColor), _hostile_to(std::move(hostileTo)), _max_aggression(maxAggro), _chance(spawnChance) {}
	/**
	 * ActorTemplate(string, ActorStats&, char, WinAPI::color, vector<FACTION>)
	 * @brief Constructs a NPC template that is always hostile to all other factions, unless the setRelationship function is used.
	 *
	 * @param templateName	- A name of type string.
	 * @param templateStats	- Ref to an ActorStats instance. (level, health, stamina, damage, visible range)
	 * @param templateChar	- A character to represent actors of this type
	 * @param templateColor	- A color to represent actors of this type
	 * @param spawnChance	- The chance that this actor template will be chosen over another. valid: (0-100)
	 * @param maxAggro		- The number of move cycles passed before this actor loses its target.
	 */
	ActorTemplate(std::string templateName, ActorStats templateStats, const char templateChar, const WinAPI::color templateColor, const int maxAggro, const unsigned int spawnChance) : _name(std::move(templateName)), _stats(std::move(templateStats)), _char(templateChar), _color(templateColor), _max_aggression(maxAggro), _chance(spawnChance) {}
};

// stats & specs of all actors -- parent struct
struct ActorBase : ActorStats {
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
	void initHostilities()
	{
		for (auto i = static_cast<int>(FACTION::PLAYER); i < static_cast<int>(FACTION::NONE); i++ ) {
			auto thisFaction{ static_cast<FACTION>(i) };
			if ( thisFaction != _faction )
				_hostileTo.push_back(thisFaction);
		}
	}
	
public:
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
	ActorBase(const FACTION myFaction, std::string myName, const Coord& myPos, const char myChar, const WinAPI::color myColor, const ActorStats& myStats) : ActorStats(myStats), _name(std::move(myName)), _faction(myFaction), _pos(myPos), _char(myChar), _color(myColor), _kill_count(0) { initHostilities(); }
	ActorBase(const FACTION myFaction, const Coord& myPos, ActorTemplate& myTemplate) : ActorStats(myTemplate._stats), _name(myTemplate._name), _faction(myFaction), _pos(myPos), _char(myTemplate._char), _color(myTemplate._color), _hostileTo(myTemplate._hostile_to), _kill_count(0) { if ( myTemplate._hostile_to.empty() && _hostileTo.empty() ) initHostilities(); }
	ActorBase(const ActorBase&) = default;
	ActorBase(ActorBase&&) = default;
	virtual ~ActorBase() = default;
	ActorBase& operator=(const ActorBase&) = default;
	ActorBase& operator=(ActorBase&&) = default;
	
	// Movement functions, these do not check if a movement was possible, they are simply a wrapper for incrementing/decrementing _pos
	void moveU() { _pos._y--; } // decrement y by 1
	void moveD() { _pos._y++; } // increment y by 1
	void moveL() { _pos._x--; } // decrement x by 1
	void moveR() { _pos._x++; } // increment x by 1
	// Moves this actor in a given direction
	void moveDir(const char dir)
	{
		switch ( dir ) {
		case 'w':
		case 'W':
			moveU();
			break;
		case 's':
		case 'S':
			moveD();
			break;
		case 'a':
		case 'A':
			moveL();
			break;
		case 'd':
		case 'D':
			moveR();
			break;
		default:break;
		}
	}
	[[nodiscard]] Coord getPosU() const { return { _pos._x, _pos._y - 1 }; }
	[[nodiscard]] Coord getPosD() const { return { _pos._x, _pos._y + 1 }; }
	[[nodiscard]] Coord getPosL() const { return { _pos._x - 1, _pos._y }; }
	[[nodiscard]] Coord getPosR() const { return { _pos._x + 1, _pos._y }; }
	// Returns the Coordinate of a tile in the given direction
	[[nodiscard]] Coord getPosDir(const char dir) const
	{
		switch ( std::tolower(dir) ) {
		case 'w':
		case 'W':
			return getPosU();
		case 's':
		case 'S':
			return getPosD();
		case 'a':
		case 'A':
			return getPosL();
		case 'd':
		case 'D':
			return getPosR();
		default:return{-1,-1};
		}
	}
	// Set this actor's relationship to another faction
	void setRelationship(const FACTION faction, const bool hostile)
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
	bool isHostileTo(const FACTION target)
	{
		for ( auto it : _hostileTo )
			if ( it == target )
				return true;
		return false;
	}
	bool isHostileTo(ActorBase* target)
	{
		for ( auto it : _hostileTo )
			if ( it == target->faction() )
				return true;
		return false;
	}
	void modColor(const WinAPI::color newColor) { _color = newColor; }
	[[nodiscard]] WinAPI::color getColor() const { return _color; }
	[[nodiscard]] auto name() const -> std::string { return _name; }
	[[nodiscard]] auto faction() const -> FACTION { return _faction; }
	[[nodiscard]] Coord pos() const { return _pos; }
	[[nodiscard]] auto isDead() const -> bool { return _dead; }
	[[nodiscard]] auto getChar() const -> char { return _char; }
	[[nodiscard]] auto getKills() const -> int { return _kill_count; }
	void addKill(const int count = 1) { _kill_count += count; }

	// stream insertion operator
	friend std::ostream& operator<<(std::ostream &os, ActorBase &a)
	{
		// set text color to actor's color
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<unsigned short>(a._color));

		// insert actor's character
		os << a._char;

		// reset text color
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 07);
		return os;
	}
};

// human player
struct Player final : ActorBase {
	checkDistanceFrom getDist;		// Functor that gets the distance from _pos to a given Coord point
	
	/** CONSTRUCTOR **
	 * Player(FACTION, string, Coord, char, WinAPI::color, int)
	 * This is the base constructor for actor types.
	 *
	 * @param myName	- My reporting name.
	 * @param myPos		- My current position as a matrix coordinate
	 * @param myChar	- My display character when inserted into a stream
	 * @param myColor	- My character's color when inserted into a stream
	 * @param myStats	- My statistics
	 */
	Player(const std::string& myName, const Coord& myPos, const char myChar, const WinAPI::color myColor, const ActorStats
		   & myStats) : ActorBase(FACTION::PLAYER, myName, myPos, myChar, myColor, myStats), getDist(_pos) {}
	Player(const Coord& myPos, ActorTemplate& myTemplate) : ActorBase(FACTION::PLAYER, myPos, myTemplate), getDist(_pos) {}
};

// base NPC actor
struct NPC : ActorBase {
private:
	int
		_MAX_AGGRO,		// Maximum aggression value
		_aggro;			// Current aggression value
protected:
	ActorBase* _target; // Current target

	/**
	 * isAfraid()
	 * @brief Returns true if this NPC's stats are too low to continue fighting
	 *
	 * @returns bool - ( true = Enemy is afraid, run away ) ( false = Enemy is not afraid, proceed with attack )
	 */
	[[nodiscard]] bool afraid() const
	{
		if ( static_cast<float>(_health) < static_cast<float>(_MAX_HEALTH) / 5.0f || static_cast<float>(_stamina) < static_cast<float>(_MAX_STAMINA) / 6.0f )
			return true;
		return false;
	}

	/**
	 * constexpr getDir(Coord&, bool)
	 * @brief Returns a direction char from a start point and end point. Called from getDirTo()
	 *
	 * @param dist		- The difference between 2 actor's positions
	 * @param invert	- When true, returns a direction away from the target
	 * @returns char	- w = up/s = down/a = left/d = right
	 */
	static constexpr char getDir(const Coord& dist, const bool invert)
	{
		if ( dist._x == 0 )	// X-axis is aligned, move vertically
			return dist._y < 0 ? invert ? 'w' : 's' : invert ? 's' : 'w';
		if ( dist._y == 0 )	// Y-axis is aligned, move horizontally
			return dist._x < 0 ? invert ? 'a' : 'd' : invert ? 'd' : 'a';
		// neither is aligned, check which axis is smaller
		if ( (dist._x < 0 ? dist._x * -1 : dist._x) > (dist._y < 0 ? dist._y * -1 : dist._y) )
			return dist._x < 0 ? invert ? 'a' : 'd' : invert ? 'd' : 'a';
		return dist._y < 0 ? invert ? 'w' : 's' : invert ? 's' : 'w';
	}

/*	static constexpr char getDir(const Coord& dist, const bool invert)
	{
		// check if either axis is aligned
		if ( dist._x == 0 || dist._y == 0 ) {
			// x axis is aligned (move vertically)
			if ( dist._x == 0 )
				return dist._y < 0 ? invert ? 'w' : 's' : invert ? 's' : 'w';
			// y axis is aligned (move horizontally)
			return dist._x < 0 ? invert ? 'a' : 'd' : invert ? 'd' : 'a';
		} // else, move on the axis with the largest distance first
		// if the absolute value of X-axis distance is larger than the absolute value of Y-axis distance
		if ( (dist._x < 0 ? dist._x * -1 : dist._x) > (dist._y < 0 ? dist._y * -1 : dist._y) )
			return dist._x < 0 ? invert ? 'w' : 's' : invert ? 's' : 'w';
		// else, Y-axis is larger
		return dist._y < 0 ? invert ? 'a' : 'd' : invert ? 'd' : 'a';
	}*/
public:
	/**
	 * NPC(FACTION, string&, Coord&, char, WinAPI::color, ActorStats&, int)
	 *
	 * @param myFaction		- This NPC's faction
	 * @param myName		- This NPC's name
	 * @param myPos			- This NPC's position
	 * @param myChar		- This NPC's display character
	 * @param myColor		- This NPC's display color
	 * @param myStats		- This NPC's stats
	 * @param MAX_AGGRO		- This NPC's maximum aggression value
	 */
	NPC(const FACTION myFaction, const std::string& myName, const Coord& myPos, const char myChar, const WinAPI::color myColor, const ActorStats& myStats, const int MAX_AGGRO) : ActorBase(myFaction, myName, myPos, myChar, myColor, myStats), _MAX_AGGRO(MAX_AGGRO), _aggro(0), _target(nullptr) {}
	/**
	 * NPC(FACTION, string&, Coord&, char, WinAPI::color, ActorStats&, int)
	 *
	 * @param myFaction		- This NPC's faction
	 * @param myPos			- This NPC's position
	 * @param myTemplate	- This NPC's templated stats
	 */
	NPC(const FACTION myFaction, const Coord& myPos, ActorTemplate& myTemplate) : ActorBase(myFaction, myPos, myTemplate), _MAX_AGGRO(myTemplate._max_aggression), _aggro(0), _target(nullptr) {}
	NPC(const NPC&) = default;
	NPC(NPC&&) = default;
	virtual ~NPC() = default;
	NPC& operator=(const NPC&) = default;
	NPC& operator=(NPC&&) = default;

	/**
	 * canSee(Coord&, int)
	 * @brief Returns true if a given point is within sight of this NPC.
	 *
	 * @param pos		- Coordinate to check
	 * @param visMod	- Optional modifier to NPC visibility, this value is added to NPC's sight range.
	 * @returns bool
	 */
	[[nodiscard]] bool canSee(const Coord& pos, const int visMod = 0) const
	{
		if ( checkDistance::get(_pos, pos) <= _visRange + visMod )
			return true;
		return false;
	}

	/**
	 * canSee(ActorBase*, int)
	 * @brief Returns true if a given actor is within sight of this NPC.
	 *
	 * @param target	- Pointer to an actor to check
	 * @param visMod	- Optional modifier to NPC visibility, this value is added to NPC's sight range.
	 * @returns bool
	 */
	[[nodiscard]] bool canSee(ActorBase* target, const int visMod = 0) const
	{
		if ( checkDistance::get(_pos, target->pos()) <= _visRange + visMod )
			return true;
		return false;
	}

	/**
	 * canSee(ActorBase*, int)
	 * @brief Returns true if a given actor is within sight of this NPC.
	 *
	 * @param target	- Pointer to an actor to check
	 * @param visMod	- Optional modifier to NPC visibility, this value is added to NPC's sight range.
	 * @returns bool
	 */
	[[nodiscard]] bool canSeeHostile(ActorBase* target, const int visMod = 0)
	{
		if ( isHostileTo(&*target) && checkDistance::get(_pos, target->pos()) <= _visRange + visMod )
			return true;
		return false;
	}

	/**
	 * canSee(int)
	 * @brief Returns true if this NPC has a target, and the target is within sight range.
	 * 
	 * @param visMod	- Optional modifier to NPC visibility, this value is added to NPC's sight range.
	 * @returns bool
	 */
	[[nodiscard]] bool canSeeTarget(const int visMod = 0) const
	{
		if ( _target != nullptr && checkDistance::get(_pos, _target->pos()) <= _visRange + visMod )
			return true;
		return false;
	}
	
	/**
	 * getDirTo(Coord&)
	 * Returns a direction char from a given target position.
	 *
	 * @param target	- The target position
	 * @param noFear	- NPC will never run away
	 * @returns char	- w = up/s = down/a = left/d = right
	 */
	[[nodiscard]] char getDirTo(const Coord& target, const bool noFear = false) const
	{
		return getDir({ _pos._x - target._x, _pos._y - target._y }, noFear ? false : afraid());
	}
	
	/**
	 * getDirTo(ActorBase*)
	 * Returns a direction char from a given target actor's position.
	 *
	 * @param target	- Pointer to the target actor
	 * @param noFear	- NPC will never run away
	 * @returns char	- w = up/s = down/a = left/d = right
	 */
	[[nodiscard]] char getDirTo(ActorBase* target, const bool noFear = false) const
	{
		if ( target != nullptr )
			return getDir({ _pos._x - target->pos()._x, _pos._y - target->pos()._y }, noFear ? false : afraid());
		return ' ';
	}
	
	/**
	 * getDirTo(ActorBase*)
	 * Returns a direction char from this NPC's current target's position.
	 *
	 * @param noFear	- NPC will never run away
	 * @returns char	- w = up/s = down/a = left/d = right
	 */
	[[nodiscard]] char getDirTo(const bool noFear = false) const
	{
		if ( _target != nullptr )
			return getDir({ _pos._x - _target->pos()._x, _pos._y - _target->pos()._y }, noFear ? false : afraid());
		return ' ';
	}
	
	// Returns true if this NPC is currently aggravated
	[[nodiscard]] bool isAggro() const
	{
		if ( _aggro == 0 )
			return false;
		return true;
	}
	// Returns this NPC's aggression value
	[[nodiscard]] int getAggro() const { return _aggro; }
	// Modify this NPC's aggression, positive adds, negative removes.
	void modAggro(const int modValue)
	{
		auto newValue{ _aggro + modValue };
		if ( newValue < 0 )
			newValue = 0;
		if ( newValue > _MAX_AGGRO )
			newValue = _MAX_AGGRO;
		_aggro = newValue;
	}
	// Make this NPC aggressive
	void maxAggro()
	{
		_aggro = _MAX_AGGRO;
	}
	// Remove this NPC's aggression
	void removeAggro() { _aggro = 0; if (_target != nullptr) removeTarget(); }
	// Decreases this NPC's aggression by 1
	void decrementAggro()
	{
		if ( _aggro - 1 <= 0 )
			_aggro = 0;
		else _aggro--;
	}

	// Returns true if this NPC has a target
	[[nodiscard]] bool hasTarget() const { return _target != nullptr; }
	// Returns this NPC's current target
	[[nodiscard]] ActorBase* getTarget() const { return _target; }
	// Set this NPC's current target
	void setTarget(ActorBase* target) 
	{ 
		if ( !isHostileTo(target->faction()) )
			setRelationship(target->faction(), true);
		_target = target;
	}
	// Remove this NPC's current target
	void removeTarget() { _target = nullptr; }
};

// enemy actor
struct Enemy final : NPC {
	/** CONSTRUCTOR **
	 * Enemy(string, Coord, char, WinAPI::color, int, int, int, int, int)
	 * Constructs a basic enemy with the given parameters.
	 *
	 * @param myName	 - My reporting name.
	 * @param myPos		 - My current position as a matrix coordinate
	 * @param myChar	 - My display character when inserted into a stream
	 * @param myColor	 - My character's color when inserted into a stream
	 * @param myLevel	 - My level, which is a multiplier for all of my stats.
	 * @param myHealth	 - My max health
	 * @param myStamina	 - My max stamina
	 * @param myDamage	 - My max damage
	 * @param myVisRange - My sight range
	 * @param MAX_AGGRO  - My maximum aggression
	 */
	Enemy(const std::string& myName, const Coord& myPos, const char myChar, const WinAPI::color myColor, const int myLevel, const int myHealth, const int myStamina, const int myDamage, const int myVisRange, const int MAX_AGGRO) : NPC(FACTION::ENEMY, myName, myPos, myChar, myColor, ActorStats(myLevel, myHealth, myStamina, myDamage, myVisRange), MAX_AGGRO) {}
	/** CONSTRUCTOR **
	 * Enemy(string, Coord, char, WinAPI::color, ActorStats&)
	 * Constructs a basic enemy with the given parameters.
	 *
	 * @param myName	- My reporting name.
	 * @param myPos		- My current position as a matrix coordinate
	 * @param myChar	- My display character when inserted into a stream
	 * @param myColor	- My character's color when inserted into a stream
	 * @param myStats	- A ref to my base statistics object
	 * @param MAX_AGGRO	- My maximum aggression
	 */
	Enemy(const std::string& myName, const Coord& myPos, const char myChar, const WinAPI::color myColor, const ActorStats
		  &
		  myStats, const int MAX_AGGRO) : NPC(FACTION::ENEMY, myName, myPos, myChar, myColor, myStats, MAX_AGGRO) {}
	Enemy(const Coord& myPos, ActorTemplate& myTemplate) : NPC(FACTION::ENEMY, myPos, myTemplate) {}
};

// neutral actor
struct Neutral final : NPC {
	/**
	 * Neutral(string, Coord, char, WinAPI::color, int, int, int, int, int)
	 * @param myName	 - My reporting name.
	 * @param myPos		 - My current position as a matrix coordinate
	 * @param myChar	 - My display character when inserted into a stream
	 * @param myColor	 - My character's color when inserted into a stream
	 * @param myLevel	 - My level, which is a multiplier for all of my stats.
	 * @param myHealth	 - My max health
	 * @param myStamina	 - My max stamina
	 * @param myDamage	 - My max damage
	 * @param myVisRange - My sight range
	 * @param MAX_AGGRO	 - My maximum possible aggression
	 */
	Neutral(const std::string& myName, const Coord& myPos, const char myChar, const WinAPI::color myColor, const int myLevel, const int myHealth, const int myStamina, const int myDamage, const int myVisRange, const int MAX_AGGRO) : NPC(FACTION::NEUTRAL, myName, myPos, myChar, myColor, ActorStats(myLevel, myHealth, myStamina, myDamage, myVisRange), MAX_AGGRO) {}
	Neutral(const std::string& myName, const Coord& myPos, const char myChar, const WinAPI::color myColor, const ActorStats
			& myStats, const int MAX_AGGRO) : NPC(FACTION::NEUTRAL, myName, myPos, myChar, myColor, myStats, MAX_AGGRO) {}
	Neutral(const Coord& myPos, ActorTemplate& myTemplate) : NPC(FACTION::NEUTRAL, myPos, myTemplate) {}
};