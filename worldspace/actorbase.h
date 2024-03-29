/**
 * @file actorbase.h
 * @author radj307
 * @brief Contains Actor Attributes, and the ActorBase & NPC virtual classes.
 */
#pragma once
// Universal attributes and templates
#include <algorithm>

#include "controls.h"
#include "Coord.h"
#include "faction.h"
#include "xRand.h"

/**
 * @struct ActorMaxStats
 * @brief Contains all actor universal maximum stats, aka their starting stats. This is the base of ActorStats.
 */
struct ActorMaxStats {
protected:
	// These are modified every level change
	int _MAX_HEALTH, _MAX_STAMINA, _MAX_DAMAGE;
	// Do not modify these
	int _BASE_HEALTH, _BASE_STAMINA, _BASE_DAMAGE;

	/** CONSTRUCTOR **
	 * ActorMaxStats(int, int, int)
	 *
	 * @param HEALTH	- My maximum health value	(Minimum is 10)
	 * @param STAMINA	- My maximum stamina value	(Minimum is 10)
	 * @param DAMAGE	- My damage modifier value	(Minimum is 10)
	 */
	ActorMaxStats(const int HEALTH, const int STAMINA, const int DAMAGE) :
		_MAX_HEALTH { HEALTH },
		_MAX_STAMINA { STAMINA },
		_MAX_DAMAGE { DAMAGE },
		_BASE_HEALTH { _MAX_HEALTH },
		_BASE_STAMINA { _MAX_STAMINA },
		_BASE_DAMAGE { _MAX_DAMAGE }
	{
		if ( HEALTH <= 0 || STAMINA <= 0 || DAMAGE <= 0 ) throw std::exception("INVALID_ACTOR_STATS");
	}
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

/**
 * @struct ActorStats
 * @brief Contains all actor universal statistics, and is the base of ActorBase.
 */
struct ActorStats : ActorMaxStats {
protected:
	int _level, _health, _stamina;	// level = Stat Multiplier
	bool _dead;						// Am I dead?
	std::string _killedBy {};		// Name of the actor who killed me
	int _visRange;					// Range in tiles that this actor can see

	/**
	 * update_stats(int)
	 * @brief Sets this actor's level to a new value, and updates their stats accordingly. This function is called by addLevel()
	 * @param newLevel		- The new level to set
	 */
	void update_stats(const int newLevel)
	{
		_level = newLevel;
		if ( _level % 3 == 0 ) {
			_MAX_HEALTH = static_cast<int>( static_cast<float>( _BASE_HEALTH ) * ( static_cast<float>( _level ) / 1.5f ) );
			_MAX_STAMINA = static_cast<int>( static_cast<float>( _BASE_STAMINA ) * ( static_cast<float>( _level ) / 1.3f ) );
			_MAX_DAMAGE = static_cast<int>( static_cast<float>( _BASE_DAMAGE ) * ( static_cast<float>( _level ) / 1.9f ) );
		}
	}

	/**
	 * restore_all_stats()
	 * @brief Restores all stats to their max values.
	 */
	void restore_all_stats()
	{
		_health = _MAX_HEALTH;
		_stamina = _MAX_STAMINA;
	}

public:
	/**
	 * ActorStats(int, int, int)
	 * @brief These values are used to determine my maximum stats.
	 * @param level		- My level
	 * @param health	- My health value
	 * @param stamina	- My stamina value
	 * @param damage	- My damage modifier
	 * @param visRange	- My sight range
	 */
	ActorStats(const int level, const int health, const int stamina, const int damage, const int visRange) :
		ActorMaxStats(health, stamina, damage), _level(level >= 1 ? level : 1), _health(_MAX_HEALTH), _stamina(_MAX_STAMINA), _dead(
		_health == 0), _visRange(visRange)
	{}

	/**
	 * setMaxHealth(unsigned int)
	 * @brief Set the max/base health values of an actor. This will also restore all stats to max.
	 */
	void setMaxHealth(const unsigned int newValue)
	{
		_MAX_HEALTH = static_cast<signed>( newValue );
		_BASE_HEALTH = _MAX_HEALTH;
		restore_all_stats();
	}
	/**
	 * setMaxStamina(unsigned int)
	 * @brief Sets the maximum & base stamina of an actor. This will also restore all stats to max.
	 * @param newValue	- The new max/base stamina value
	 */
	void setMaxStamina(const unsigned int newValue)
	{
		_MAX_STAMINA = static_cast<signed>( newValue );
		_BASE_STAMINA = _MAX_STAMINA;
		restore_all_stats();
	}
	/**
	 * setMaxDamage(unsigned int)
	 * @brief Set the maximum damage value of an actor. This will also restore all stats to max.
	 * @param newValue	- The new max damage value.
	 */
	void setMaxDamage(const unsigned int newValue)
	{
		_MAX_DAMAGE = static_cast<signed>( newValue );
		_BASE_DAMAGE = _MAX_DAMAGE;
		restore_all_stats();
	}
	/**
	 * killedBy(string&)
	 * @brief Optionally sets, and returns the name of this actor's killer.
	 * @param killer	- (Default: {}}) When not empty, sets the name of this actor's killer to the given string.
	 * @returns string
	 */
	std::string killedBy(const std::string& killer = {})
	{
		if ( !killer.empty() )
			_killedBy = killer;
		return _killedBy;
	}
	[[nodiscard]] int getLevel() const { return _level; } ///< @brief Returns the current level of this instance. @returns int
	[[nodiscard]] auto getVis() const -> int { return _visRange; } ///< @brief Returns the current visibility range of this instance. @returns int
	void addLevel() { update_stats(++_level); } ///< @brief Increase the current level of this instance by one.
	void subLevel() { update_stats(_level > 1 ? --_level : _level = 1); } ///< @brief Decrease the current level of this instance by one.
	[[nodiscard]] int getHealth() const { return _health; } ///< @brief Returns the current health of this instance. @returns int
	[[nodiscard]] int getStamina() const { return _stamina; } ///< @brief Returns the current stamina value of this instance. @returns int
	/**
	 * setHealth(int)
	 * @brief Sets the actor's health to a new value, and automatically sets the dead flag if it is below 0.
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
	 * @brief Modifies the actor's health value, and automatically sets the dead flag if it drops below 0.
	 * @param modValue	- The amount to modify health by, negative removes, positive adds.
	 * @returns int		- The new health value
	 */
	int modHealth(const int modValue)
	{
		const auto newHealth { _health + modValue };
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
	 * setStamina(int)
	 * @brief Sets the actor's stamina to a new value.
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
	 * @brief Modifies the actor's stamina value.
	 * @param modValue	- The amount to modify stamina by, negative removes, positive adds.
	 * @returns int		- The new stamina value
	 */
	int modStamina(const int modValue)
	{
		const auto newStamina { _stamina + modValue };
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

/**
 * @struct ActorTemplate
 * @brief Contains all of the required values to initialize an Actor of any level, except for the starting position. An instance of this can be passed to the constructor of any actor to quickly create multiple similar actors.
 */
struct ActorTemplate {
	std::string _name;					///< This instance's name.
	ActorStats _stats;					///< This instance's stats.
	char _char;							///< This instance's display character.
	unsigned short _color;				///< This instance's display color.
	std::vector<FACTION> _hostile_to;	///< This instance's faction relationships.
	int _max_aggression;				///< This instance's maximum aggression value.
	float _chance;						///< This instance's spawn chance.

	/**
	 * ActorTemplate(string, ActorStats&, char, unsigned short, vector<FACTION>)
	 * @brief Constructs a human player actor template.
	 * @param templateName	- A name of type string.
	 * @param templateStats	- Ref to an ActorStats instance. (level, health, stamina, damage, visible range)
	 * @param templateChar	- A character to represent actors of this type
	 * @param templateColor	- A color to represent actors of this type
	 */
	ActorTemplate(std::string templateName, ActorStats templateStats, const char templateChar, const unsigned short templateColor) : _name(std::move(templateName)), _stats(std::move(templateStats)), _char(templateChar), _color(templateColor), _max_aggression(0), _chance(100.0f) {}
	/**
	 * ActorTemplate(string, ActorStats&, char, unsigned short, vector<FACTION>)
	 * @brief Constructs a NPC actor template. This template type is used to randomly generate NPCs
	 * @param templateName	- A name of type string.
	 * @param templateStats	- Ref to an ActorStats instance. (level, health, stamina, damage, visible range)
	 * @param templateChar	- A character to represent actors of this type
	 * @param templateColor	- A color to represent actors of this type
	 * @param hostileTo		- A list of factions that actors of this type are hostile to
	 * @param spawnChance	- The chance that this actor template will be chosen over another. valid: (0-100)
	 * @param maxAggro		- The number of move cycles passed before this actor loses its target.
	 */
	ActorTemplate(std::string templateName, ActorStats templateStats, const char templateChar, const unsigned short templateColor, std::vector<FACTION> hostileTo, const int maxAggro, const float spawnChance) : _name(std::move(templateName)), _stats(std::move(templateStats)), _char(templateChar), _color(templateColor), _hostile_to(std::move(hostileTo)), _max_aggression(maxAggro), _chance(spawnChance) {}
	/**
	 * ActorTemplate(string, ActorStats&, char, unsigned short, vector<FACTION>)
	 * @brief Constructs a NPC template that is always hostile to all other factions, unless the setRelationship function is used.
	 * @param templateName	- A name of type string.
	 * @param templateStats	- Ref to an ActorStats instance. (level, health, stamina, damage, visible range)
	 * @param templateChar	- A character to represent actors of this type
	 * @param templateColor	- A color to represent actors of this type
	 * @param spawnChance	- The chance that this actor template will be chosen over another. valid: (0-100)
	 * @param maxAggro		- The number of move cycles passed before this actor loses its target.
	 */
	ActorTemplate(std::string templateName, ActorStats templateStats, const char templateChar, const unsigned short templateColor, const int maxAggro, const float spawnChance) : _name(std::move(templateName)), _stats(std::move(templateStats)), _char(templateChar), _color(templateColor), _max_aggression(maxAggro), _chance(spawnChance) {}
};

#pragma endregion  ACTOR_ATTRIBUTES
// Base class of all actors
#pragma region ACTOR_BASE
/**
 * @struct ActorBase
 * @brief This is the base virtual object of every actor in the game. An ActorBase* parameter in a function means any actor can be operated on by the function.
 */
struct ActorBase : ActorStats {
protected:
	std::string _name;					///< This actor's name.
	FACTION _faction;					///< This actor's Faction.
	Coord _pos;							///< This actor's current position.
	char _char;							///< This actor's display character.
	unsigned short _color;				///< The color of this actor's display character.
	std::vector<FACTION> _hostileTo;	///< Factions this actor is hostile to.
	int _kill_count;					///< This actor's kill count / experience.

private:
	/**
	 * initHostilities()
	 * @brief Initializes this actor's hostile faction list with all factions marked as hostile.
	 */
	void initHostilities()
	{
		for ( auto i = static_cast<int>( FACTION::PLAYER ); i < static_cast<int>( FACTION::NONE ); i++ ) {
			auto thisFaction { static_cast<FACTION>( i ) };
			if ( thisFaction != _faction )
				_hostileTo.push_back(thisFaction);
		}
	}

public:
	/** CONSTRUCTOR **
	 * ActorBase(FACTION, string, Coord&, char, unsigned short, ActorStats&)
	 * @brief Construct an actor from an ActorStats instance.
	 * @param myFaction	 - My faction / group of actors.
	 * @param myName	 - My reporting name.
	 * @param myPos		 - My current position as a matrix coordinate
	 * @param myChar	 - My display character when inserted into a stream
	 * @param myColor	 - My character's color when inserted into a stream
	 * @param myStats	 - My base statistics
	 */
	ActorBase(const FACTION myFaction, std::string myName, const Coord& myPos, const char myChar, const unsigned short myColor, const ActorStats& myStats) : ActorStats(myStats), _name(std::move(myName)), _faction(myFaction), _pos(myPos), _char(myChar), _color(myColor), _kill_count(0) { initHostilities(); }
	/** CONSTRUCTOR **
	 * ActorBase(FACTION, Coord&, ActorTemplate&)
	 * @brief Construct an actor from a template.
	 * @param myFaction	 - My faction / group of actors.
	 * @param myPos		 - My current position as a matrix coordinate
	 * @param myTemplate - My templated stats
	 */
	ActorBase(const FACTION myFaction, const Coord& myPos, ActorTemplate& myTemplate) : ActorStats(myTemplate._stats), _name(myTemplate._name), _faction(myFaction), _pos(myPos), _char(myTemplate._char), _color(myTemplate._color), _hostileTo(myTemplate._hostile_to), _kill_count(0) { if ( myTemplate._hostile_to.empty() && _hostileTo.empty() ) initHostilities(); }
#pragma region DEF
	ActorBase(const ActorBase&) = default;
	ActorBase(ActorBase&&) = default;
	~ActorBase() override = default;
	ActorBase& operator=(const ActorBase&) = default;
	ActorBase& operator=(ActorBase&&) = default;
#pragma endregion DEF

	/**
	 * moveU()
	 * @brief Set this actor's position to the tile above.
	 */
	void moveU() { _pos._y--; }
	/**
	 * moveD()
	 * @brief Set this actor's position to the tile below.
	 */
	void moveD() { _pos._y++; }
	/**
	 * moveL()
	 * @brief Set this actor's position to the tile to the left.
	 */
	void moveL() { _pos._x--; }
	/**
	 * moveR()
	 * @brief Set this actor's position to the tile to the right.
	 */
	void moveR() { _pos._x++; }
	/**
	 * moveDir()
	 * @brief Set this actor's position to the tile in a given direction.
	 * @param dir	- A direction char from the controlset
	 */
	void moveDir(const char dir)
	{
		if ( dir == _current_control_set->_KEY_UP )
			moveU();
		else if ( dir == _current_control_set->_KEY_RIGHT )
			moveR();
		else if ( dir == _current_control_set->_KEY_DOWN )
			moveD();
		else if ( dir == _current_control_set->_KEY_LEFT )
			moveL();
	}
	/**
	 * getPosU()
	 * @brief Returns the coordinate of the tile above this actor.
	 * @returns Coord
	 */
	[[nodiscard]] Coord getPosU() const { return { _pos._x, _pos._y - 1 }; }
	/**
	 * getPosD()
	 * @brief Returns the coordinate of the tile below this actor.
	 * @returns Coord
	 */
	[[nodiscard]] Coord getPosD() const { return { _pos._x, _pos._y + 1 }; }
	/**
	 * getPosL()
	 * @brief Returns the coordinate of the tile to the left of this actor.
	 * @returns Coord
	 */
	[[nodiscard]] Coord getPosL() const { return { _pos._x - 1, _pos._y }; }
	/**
	 * getPosR()
	 * @brief Returns the coordinate of the tile to the right of this actor.
	 * @returns Coord
	 */
	[[nodiscard]] Coord getPosR() const { return { _pos._x + 1, _pos._y }; }
	/**
	 * getPosDir()
	 * @brief Returns the coordinate of the tile in the given direction, in relation to the position of this actor.
	 * @returns Coord
	 */
	[[nodiscard]] Coord getPosDir(const char dir) const
	{
		if ( dir == _current_control_set->_KEY_UP )
			return getPosU();
		if ( dir == _current_control_set->_KEY_RIGHT )
			return getPosR();
		if ( dir == _current_control_set->_KEY_DOWN )
			return getPosD();
		if ( dir == _current_control_set->_KEY_LEFT )
			return getPosL();
		return{ -1, -1 }; // undefined
	}

	/**
	 * setRelationship(FACTION, bool)
	 * @brief Modify this actor's relationship to a given faction.
	 * @param faction	- A faction
	 * @param hostile	- ( true = This actor will be hostile towards the given faction ) ( false = This actor will be passive towards the given faction )
	 */
	void setRelationship(const FACTION faction, const bool hostile)
	{
		for ( size_t it { 0 }; it < _hostileTo.size(); it++ ) {
			if ( _hostileTo.at(it) == faction ) {
				if ( !hostile ) // remove faction hostility
					_hostileTo.erase(_hostileTo.begin() + it);
				else return; // faction is already hostile
			}
		}
		// Add this faction as hostile
		_hostileTo.push_back(faction);
	}

	/**
	 * isHostileTo(FACTION)
	 * @brief Check if this actor is hostile to a given faction.
	 * @param target	- A faction
	 * @returns bool	- ( true = this actor is hostile to target ) ( false = this actor is not hostile to target )
	 */
	bool isHostileTo(const FACTION target) { return std::ranges::any_of(_hostileTo.begin(), _hostileTo.end(), [&target](FACTION& it) -> bool { return it == target; }); }

	/**
	 * isHostileTo(ActorBase*)
	 * @brief Check if this actor is hostile to the given target actor.
	 * @param target	- Ptr to an actor
	 * @returns bool	- ( true = this actor is hostile to target ) ( false = this actor is not hostile to target )
	 */
	bool isHostileTo(ActorBase* target) { return std::ranges::any_of(_hostileTo.begin(), _hostileTo.end(), [&target](FACTION& it) -> bool { return it == target->faction(); }); }

	/**
	 * setColor(unsigned short)
	 * @brief Set this actor's color.
	 * @param newColor	- New color
	 */
	void setColor(const unsigned short newColor) { _color = newColor; }
	/**
	 * getColor()
	 * @brief Returns this actor's current display color.
	 * @returns unsigned short
	 */
	[[nodiscard]] unsigned short getColor() const { return _color; }
	/**
	 * name()
	 * @brief Returns this actor's name.
	 * @returns string
	 */
	[[nodiscard]] auto name() const -> std::string { return _name; }
	/**
	 * faction()
	 * @brief Returns this actor's faction.
	 * @returns FACTION
	 */
	[[nodiscard]] auto faction() const -> FACTION { return _faction; }
	/**
	 * pos()
	 * @brief Returns this actor's current position.
	 * @returns Coord
	 */
	[[nodiscard]] Coord pos() const { return _pos; }
	/**
	 * getPosPtr()
	 * @brief Returns a pointer to this actor position value.
	 * @returns Coord*
	 */
	[[nodiscard]] Coord* getPosPtr() { return &_pos; }
	/**
	 * isDead()
	 * @brief Checks if this actor is dead.
	 * @returns bool	- ( true = This actor is dead ) ( false = This actor is alive )
	 */
	[[nodiscard]] bool isDead() const { return _dead; }
	/**
	 * getChar()
	 * @brief Returns this actor's display character.
	 * @returns char
	 */
	[[nodiscard]] char getChar() const { return _char; }
	/**
	 * getKills()
	 * @brief Returns this actor's kill count.
	 * @returns int
	 */
	[[nodiscard]] int getKills() const { return _kill_count; }
	/**
	 * addKill()
	 * @brief Adds to this actor's kill count.
	 * @param count	- (Default: 1) The number of kills to add.
	 * @returns int	- This actor's new kill count.
	 */
	int addKill(const int count = 1) { return count > 0 ? _kill_count += count : _kill_count; }

	/**
	 * print()
	 * @brief Prints this actor's colorized display character at the current cursor position.
	 */
	void print() const
	{
		sys::colorSet(_color);
		printf("%c", _char);
		sys::colorReset();
	}
};
/**
 * @struct NPC
 * @brief This is the base of all NPC actors in the game.
 */
struct NPC : ActorBase {
private:
	int
		_MAX_AGGRO,		///< @brief Maximum aggression value
		_aggro;			///< @brief Current aggression value
	unsigned int _blind { 0 };
protected:
	ActorBase* _target; ///< @brief Current target, the NPC will attempt to move towards this actor and kill them.

	/**
	 * isAfraid()
	 * @brief Returns true if this NPC's stats are too low to continue fighting
	 * @return true		- NPC is afraid because their stats are low, they should run away.
	 * @return false	- NPC is not afraid.
	 */
	[[nodiscard]] bool afraid() const
	{
		return _blind > 0 || static_cast<float>( _health ) < static_cast<float>( _MAX_HEALTH ) / 6.0f || static_cast<float>( _stamina ) <
			static_cast<float>( _MAX_STAMINA ) / 6.0f;
	}

	/**
	 * getDir(Coord&, bool)
	 * @brief Returns a direction char from a start point and end point. Called from getDirTo()
	 * @param dist		- The non-absolute difference between 2 actor's positions.
	 * @param invert	- When true, returns a direction away from the target
	 * @returns char	- w = up/s = down/a = left/d = right
	 */
	[[nodiscard]] static char getDir(const Coord& dist, const bool invert)
	{
		const auto x_axis { [&invert](const int x_dist) -> char {
			return x_dist < 0 // Return X-axis direction
				? invert ? _current_control_set->_KEY_LEFT : _current_control_set->_KEY_RIGHT
				: invert ? _current_control_set->_KEY_RIGHT : _current_control_set->_KEY_LEFT;
		} };
		const auto y_axis { [&invert](const int y_dist) -> char {
			return y_dist < 0 // Return Y-axis direction
				? invert ? _current_control_set->_KEY_UP : _current_control_set->_KEY_DOWN
				: invert ? _current_control_set->_KEY_DOWN : _current_control_set->_KEY_UP;
		} };
		const auto absX { abs(dist._x) }, absY { abs(dist._y) };
		/// NPC is aligned with target
		if ( absX == 0 ) // aligned horizontally
			return y_axis(dist._y);
		if ( absY == 0 ) // aligned vertically
			return x_axis(dist._x);
		/// NPC is nearly aligned with target
		if ( absX <= 2 && absY > 2 )
			return y_axis(dist._y);
		if ( absY <= 2 && absX > 2 )
			return x_axis(dist._y);
		if ( absX < absY )
			return x_axis(dist._x);
		return y_axis(dist._y);
	}

public:
#pragma region DEF
	NPC(const NPC&) = default;            ///< @brief Default copy constructor.
	NPC(NPC&&) = default;                 ///< @brief Default move constructor.
	~NPC() override = default;            ///< @brief Virtual destructor.
	NPC& operator=(const NPC&) = default; ///< @brief Default copy operator.
	NPC& operator=(NPC&&) = default;      ///< @brief Default move operator.
#pragma endregion DEF

	/**
	 * NPC(FACTION, string&, Coord&, char, unsigned short, ActorStats&, int)
	 * @brief Constructor that requires all values to be given an instantiation.
	 * @param myFaction		- This NPC's faction.
	 * @param myName		- This NPC's name.
	 * @param myPos			- This NPC's position.
	 * @param myChar		- This NPC's display character.
	 * @param myColor		- This NPC's display color.
	 * @param myStats		- This NPC's stats.
	 * @param MAX_AGGRO		- This NPC's maximum aggression value.
	 */
	NPC(const FACTION myFaction, const std::string& myName, const Coord& myPos, const char myChar, const unsigned short myColor, const ActorStats& myStats, const int MAX_AGGRO) : ActorBase(myFaction, myName, myPos, myChar, myColor, myStats), _MAX_AGGRO(MAX_AGGRO), _aggro(0), _target(nullptr) {}
	/**
	 * NPC(FACTION, string&, Coord&, char, unsigned short, ActorStats&, int)
	 * @brief Constructor that takes a ref to an ActorTemplate instance.
	 * @param myFaction		- This NPC's faction.
	 * @param myPos			- This NPC's position.
	 * @param myTemplate	- This NPC's templated stats.
	 */
	NPC(const FACTION myFaction, const Coord& myPos, ActorTemplate& myTemplate) : ActorBase(myFaction, myPos, myTemplate), _MAX_AGGRO(myTemplate._max_aggression), _aggro(0), _target(nullptr) {}

#pragma region BLIND
	/**
	 * isBlind()
	 * @brief Check if this NPC is blind.
	 * @return true		- NPC is currently blind.
	 * @return false	- NPC is not blind.
	 */
	[[nodiscard]] bool isBlind() const { return _blind != 0; }
	/**
	 * decrement_blind()
	 * @brief Reduce this NPC's blind counter by one.
	 * @returns unsigned int	- Blind value after removing 1 from it.
	 */
	unsigned int decrement_blind() { if ( _blind > 0 ) return --_blind; return 0; }
	/**
	 * getBlind()
	 * @brief Retrieve this NPC's blindness counter value.
	 * @returns unsigned int	- This NPC's blindness counter.
	 */
	[[nodiscard]] unsigned int getBlind() const { return _blind; }
#pragma endregion BLIND
#pragma region CAN_SEE
	/**
	 * canSee(Coord&, int)
	 * @brief Returns true if a given point is within sight of this NPC.
	 * @param pos		- Coordinate to check
	 * @param visMod	- (Default: 0) Modifier to NPC visibility, this value is added to NPC's sight range.
	 * @returns bool	- ( true = pos is within this NPC's visibility range ) ( false = pos is not visible )
	 */
	[[nodiscard]] bool canSee(const Coord& pos, const int visMod = 0) const { return _blind == 0 && checkDistance::get(_target->pos(), pos, _visRange + visMod); }

	/**
	 * canSee(ActorBase*, int)
	 * @brief Returns true if a given actor is within sight of this NPC.
	 * @param target	- Pointer to an actor to check
	 * @param visMod	- (Default: 0) Modifier to NPC visibility, this value is added to NPC's sight range.
	 * @returns bool	- ( true = NPC's target is within its visibility range ) ( false = NPC cannot see its target )
	 */
	[[nodiscard]] bool canSeeHostile(ActorBase* target, const int visMod = 0) { return _blind == 0 && isHostileTo(&*target) && checkDistance::get(target->pos(), _pos, _visRange + visMod); }

	/**
	 * canSee(int)
	 * @brief Returns true if this NPC has a target, and the target is within sight range.
	 * @param visMod	- (Default: 0) Modifier to NPC visibility, this value is added to NPC's sight range.
	 * @returns bool	- ( true = NPC's target is within its visibility range ) ( false = NPC cannot see its target )
	 */
	[[nodiscard]] bool canSeeTarget(const int visMod = 0) const { return _blind == 0 && _target != nullptr && checkDistance::get(_target->pos(), _pos, _visRange + visMod); }
#pragma endregion CAN_SEE
#pragma region DIRECTIONS
	/**
	 * getDirTo(Coord&)
	 * @brief Returns a direction char from a given target position.
	 * @param target	- The target position
	 * @param noFear	- (Default: false) When true, NPC will never run away
	 * @returns char	- w = up/s = down/a = left/d = right
	 */
	[[nodiscard]] char getDirTo(const Coord& target, const bool noFear = false) const { return getDir({ _pos._x - target._x, _pos._y - target._y }, noFear ? false : afraid()); }

	/**
	 * getDirTo(ActorBase*)
	 * @brief Returns a direction char from a given target actor's position.
	 * @param target	- Pointer to a target actor
	 * @param noFear	- (Default: false) When true, NPC will never run away
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
	 * @param noFear	- (Default: false) When true, NPC will never run away
	 * @returns char	- w = up/s = down/a = left/d = right
	 */
	[[nodiscard]] char getDirTo(const bool noFear = false) const { return ( _target != nullptr ? getDir({ _pos._x - _target->pos()._x, _pos._y - _target->pos()._y }, noFear ? false : afraid()) : ' ' ); }
#pragma endregion DIRECTIONS
#pragma region AGGRESSION
	[[nodiscard]] bool isAggro() const { return _aggro > 0; } ///< @brief Check if this NPC is aggravated. @return true - NPC is aggravated. @return false - NPC is not aggravated.

	[[nodiscard]] int getAggro() const { return _aggro; } ///< @brief Returns a copy of this NPC's current aggression value. @returns int

	/**
	 * modAggro(int)
	 * @brief Modifies NPC aggression by a specified value.
	 * @param modValue	- Positive values add aggression, negative removes it.
	 */
	void modAggro(const int modValue)
	{
		auto newValue { _aggro + modValue };
		if ( newValue < 0 )
			newValue = 0;
		if ( newValue > _MAX_AGGRO )
			newValue = _MAX_AGGRO;
		_aggro = newValue;
	}

	void maxAggro() { _aggro = _MAX_AGGRO; } ///< @brief Sets this NPC's aggression to maximum, does not check for targets.

	/**
	 * maxAggro()
	 * @brief Sets a target for this NPC, and raises their aggression to max.
	 * @param target	- Actor to set as new target
	 * @returns bool	- ( true = Aggression & Target set successfully ) ( false = failed to set target )
	 */
	bool setTargetMaxAggro(ActorBase* target)
	{
		// If target was set successfully, set aggression to max & return true
		if ( setTarget(target) ) {
			_aggro = _MAX_AGGRO;
			return true;
		}
		return false;
	}

	/**
	 * removeAggro()
	 * @brief Sets this NPC's aggression to 0, and sets target to nullptr.
	 */
	void removeAggro() { _aggro = 0; removeTarget(); }

	/**
	 * decrementAggro()
	 * @brief Decreases this NPC's aggression by 1.
	 */
	void decrementAggro() { if ( _aggro > 0 ) --_aggro; }
#pragma endregion AGGRESSION
#pragma region TARGET
	/**
	 * hasTarget()
	 * @brief Checks if this NPC has a target. If NPC's target is dead, this function removes their target & returns false.
	 * @returns bool	- ( true = NPC has a valid target ) ( false = NPC does not have a target )
	 */
	[[nodiscard]] bool hasTarget()
	{
		if ( _target == nullptr )
			return false;
		if ( _target->isDead() ) { // if NPC's target is dead, remove target & return false
			removeTarget();
			return false;
		}
		return true;
	}

	/**
	 * getTarget()
	 * @brief Returns a pointer to this NPC's target.
	 * @returns ActorBase*	- ( nullptr = NPC does not have a target. )
	 */
	[[nodiscard]] ActorBase* getTarget() const { return _target; }

	/**
	 * setTarget(ActorBase*)
	 * @brief Sets a target for this NPC, this function should be called from setTargetMaxAggro() instead.
	 * @param target	- A pointer to the target
	 * @returns bool	- ( true = Set target successfully ) ( false = failed, target is same faction, or nullptr )
	 */
	[[nodiscard]] bool setTarget(ActorBase* target)
	{
		// don't set target if the given target is from the same faction
		if ( target->faction() == _faction )
			return false;
		// if there is already a target set, remove it first
		if ( _target != nullptr ) _target = nullptr;
		// if NPC is not hostile to the target's faction, make them hostile to it
		if ( !isHostileTo(target->faction()) )
			setRelationship(target->faction(), true);
		// set the target
		_target = target;
		return true;
	}

	/**
	 * removeTarget()
	 * @brief Sets this NPC's target to nullptr.
	 */
	void removeTarget() { _target = nullptr; }
#pragma endregion TARGET
};