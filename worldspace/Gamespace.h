/**
 * game.h
 * Represents the game, with a worldspace & actors.
 * Contains the Gamespace class, which is a container for cells, and actors.
 * by radj307
 */
#pragma once
#include "cell.h"
#include "actor.h"
#include "settings.h"
#include "WinAPI.h"

/**
 * struct GameRules
 * Gamespace requires an instance of GameRules to construct, these are generic settings that may be used by a game to allow user-customization.
 */
struct GameRules {
	// CELL / WORLD
	bool _walls_always_visible{ true };		// When true, wall tiles are always visible to the player.

	// TRAPS
	int _trap_dmg{ 20 };					// the amount of health an actor loses when they step on a trap
	bool _trap_percentage{ true };			// whether the _trap_dmg amount is static, or a percentage of max health

	// ATTACKS
	int _attack_cost_stamina{ 15 };			// The amount of stamina used when attacking.

	// PLAYER
	ActorTemplate _player_template{ ActorStats(1, 100, 100, 45, 4), '$', WinAPI::color::green };

	// ENEMIES
	std::vector<ActorTemplate> _enemy_template{
		{ ActorStats(1, 40, 40, 10, 3), 'Y', WinAPI::color::yellow },
		{ ActorStats(2, 50, 50, 15, 4), 'T', WinAPI::color::red },
		{ ActorStats(3, 50, 50, 20, 5), 'T', WinAPI::color::magenta },
	};
	int
		_enemy_count{ 10 },					// how many enemies are present when the game starts
		_enemy_move_chance{ 4 },			// 1 in (this) chance of a hostile moving each cycle (range is 0-this)
		_enemy_aggro_distance{ 4 },			// Determines from how far away enemies notice the player and become aggressive
		_enemy_aggro_duration{ 6 };			// Determines how long enemies will remain aggressive

	// LEVELS
	int _level_up_kills{ 2 };
	int _level_up_mult{ 2 };	// Multiplier for the level up kills threshold, this is applied after every level

	// PASSIVE EFFECTS
	int
		_regen_health{ 5 },
		_regen_stamina{ 10 };
};

/**
 * class Gamespace
 * Contains all of the game-related functions required for running a game. Does not contain any display functions, use external FrameBuffer.
 */
class Gamespace {
	// worldspace cell
	Cell _world;
	// Reference to the game's ruleset
	GameRules &_ruleset;
	// Randomization engine
	tRand _rng;

	// player character
	Player _player;
	// generic enemies
	std::vector<Enemy> _hostile;
	// When this is true, the player wins.
	bool _allEnemiesDead{ false };

	/**
	 * generateEnemies(const int)
	 * Create a number of hostiles with random attributes
	 * 
	 * @param count	- The number of hostiles to generate
	 */
	inline void generateEnemies(const int count)
	{
		int i = 0;
		for ( Coord pos{ _rng.get(_world._max._x - 1, 1), _rng.get(_world._max._y - 1, 1) }; i < count; pos = Coord(_rng.get(_world._max._x - 1, 1), _rng.get(_world._max._y - 1, 1)) ) {
			if ( _world.get(pos)._canMove && !_world.get(pos)._isTrap ) {
				unsigned int sel{ _rng.get(100u, 0u) };

				if ( sel <= 50 )
					sel = 0;
				else if ( sel <= 80 )
					sel = 1;
				else if ( sel <= 100 )
					sel = 2;

				_hostile.push_back({ "Enemy", pos, _ruleset._enemy_template.at(sel) });
				i++;
			}
		}
	}

	/**
	 * isAfraid(Enemy*)
	 * Returns true if the given enemy's stats are too low to proceed with an attack on the player
	 * 
	 * @param e		 - Pointer to an Enemy instance
	 * @returns bool - ( true = Enemy is afraid, run away ) ( false = Enemy is not afraid, proceed with attack )
	 */
	inline bool isAfraid(ActorBase* a)
	{
		if ( a != nullptr && ((a->getHealth() < (a->getMaxHealth() / 4)) || (a->getStamina() < _ruleset._attack_cost_stamina)) ) 
			return true;
		return false;
	}

	/**
	 * regen(ActorBase*)
	 * Increases an actor's stats by the relevant value in ruleset.
	 * 
	 * @param a	- Pointer to an actor
	 */
	inline void regen(ActorBase* a)
	{
		if ( a != nullptr ) {
			a->modHealth(_ruleset._regen_health);
			a->modStamina(_ruleset._regen_stamina);
		}
	}

	inline void level_up(ActorBase* a)
	{
		if ( (a != nullptr) && (a->getKills() >= ((a->getLevel() * _ruleset._level_up_mult) * _ruleset._level_up_kills)) )
			a->addLevel();
	}

	/**
	 * intToDir(int)
	 * Converts an integer to a direction char. Used by NPCs to select a movement direction consistent with the player controls.
	 * 
	 * @param i			- ( 0 = 'w'/up ) ( 1 = 's'/down ) ( 2 = 'a'/left ) ( 3 = 'd'/right ) ( other = ' ' )
	 * @returns char	- w/a/s/d
	 */
	inline char intToDir(int i)
	{
		switch ( i ) {
		case 0:return 'w';
		case 1:return 's';
		case 2:return 'a';
		case 3:return 'd';
		default:return ' ';
		}
	}

	/**
	 * getDirTo(Coord, Coord)
	 * Returns a direction char from a start point and end point.
	 * 
	 * @param pos		- The starting/current position
	 * @param target	- The target position
	 * @param invert	- When true, returns a direction away from the target
	 * @returns char	- w = up/s = down/a = left/d = right
	 */
	inline char getDirTo(Coord pos, Coord target, bool invert = false)
	{
		int distX{ pos._x - target._x }, distY{ pos._y - target._y };
		// reduce large X distances to -1 or 1
		if ( distX < -1 )		distX = -1;
		else if ( distX > 1 )	distX = 1;
		// reduce large Y distances to -1 or 1
		if ( distY < -1 )		distY = -1;
		else if ( distY > 1 )	distY = 1;
		// select a direction
		if ( distX == 0 && !invert )		return { (distY < 0) ? ('s') : ('w') }; // check if X-axis is aligned
		else if ( distY == 0 && !invert )	return { (distX < 0) ? ('d') : ('a') }; // check if Y-axis is aligned
		else if ( distX == 0 && invert )	return { (distY < 0) ? ('w') : ('s') }; // check if X-axis is aligned (inverted)
		else if ( distY == 0 && invert )	return { (distX < 0) ? ('a') : ('d') }; // check if Y-axis is aligned (inverted)
		else { // neither axis is aligned, select a random direction
			switch ( _rng.get(1, 0) ) {
			case 0: // move on Y-axis
				if ( !invert )				return { (distY < 0) ? ('s') : ('w') };
				else						return { (distY < 0) ? ('w') : ('s') };
				break;
			case 1: // move on X-axis
				if ( !invert )				return { (distX < 0) ? ('d') : ('a') };
				else						return { (distX < 0) ? ('a') : ('d') };
				break;
			default:return(' ');
			}
		}
	}

	/**
	 * canMove(Coord)
	 * Returns true if the target position can be moved to.
	 *
	 * @param pos	 - Target position
	 * @returns bool - ( false = cannot move ) ( true = can move )
	 */
	inline bool canMove(Coord pos)
	{
		return ((_world.get(pos)._canMove && getActorAt(pos) == nullptr) ? (true) : (false));
	}
	/**
	 * canMove(int, int)
	 * Returns true if the target position can be moved to.
	 * 
	 * @param posX	 - Target X position
	 * @param posY	 - Target Y position
	 * @returns bool - ( false = cannot move ) ( true = can move )
	 */
	inline bool canMove(int posX, int posY)
	{
		return ((_world.get(posX, posY)._canMove && getActorAt(posX, posY) == nullptr) ? (true) : (false));
	}

	/**
	 * move(ActorBase*, char)
	 * Attempts to move the target actor to an adjacent tile, and processes trap logic.
	 *
	 * @param actor	 - A pointer to the target actor
	 * @param dir	 - (w = up / s = down / a = left / d = right) all other characters are ignored.
	 * @returns bool - ( true = moved successfully ) ( false = did not move )
	 */
	inline bool move(ActorBase* actor, char dir)
	{
		if ( actor != nullptr ) {
			bool didMove{ false }; // boolean to 
			ActorBase* target{ nullptr }; // declare a pointer to potential attack target
			int attackCode{ 0 }; // integer to hold the return val of potential attack
			switch ( dir ) {
			case 'W':
			case 'w':
				target = getActorAt(actor->pos()._x, actor->pos()._y - 1); // define the target
				if ( target != nullptr )
					attackCode = attack(actor, target);
				else if ( attackCode == 1 || canMove(actor->pos()._x, actor->pos()._y - 1) ) {
					actor->moveU();
					didMove = true;
				}
				break;
			case 'S':
			case 's':
				target = getActorAt(actor->pos()._x, actor->pos()._y + 1); // define the target
				if ( target != nullptr )
					attackCode = attack(actor, target);
				else if ( attackCode == 1 || canMove(actor->pos()._x, actor->pos()._y + 1) ) {
					actor->moveD();
					didMove = true;
				}
				break;
			case 'A':
			case 'a':
				target = getActorAt(actor->pos()._x - 1, actor->pos()._y); // define the target
				if ( target != nullptr )
					attackCode = attack(actor, target);
				else if ( attackCode == 1 || canMove(actor->pos()._x - 1, actor->pos()._y) ) {
					actor->moveL();
					didMove = true;
				}
				break;
			case 'D':
			case 'd':
				target = getActorAt(actor->pos()._x + 1, actor->pos()._y); // define the target
				if ( target != nullptr )
					attackCode = attack(actor, target);
				else if ( attackCode == 1 || canMove(actor->pos()._x + 1, actor->pos()._y) ) {
					actor->moveR();
					didMove = true;
				}
				break;
			default:return didMove; // if the given char does not match a direction, return to avoid processing the trap logic twice.
			}
			// if this tile is a trap
			if ( _world.get(actor->pos())._isTrap ) {
				switch ( _ruleset._trap_percentage ) {
				case true:
					actor->modHealth(-static_cast<int>(static_cast<float>(actor->getMaxHealth()) * (static_cast<float>(_ruleset._trap_dmg) / 100.0f)));
					break;
				default:
					actor->modHealth(-_ruleset._trap_dmg);
					break;
				}
			}
			return didMove;
		}
		else throw std::exception("Cannot move() a nullptr!");
	}

	/**
	 * attack(ActorBase*, ActorBase*)
	 * Allows an actor to attack another actor.
	 * 
	 * @param attacker	- A pointer to the attacking actor
	 * @param target	- A pointer to the actor being attacked
	 * @returns int		- ( -1 = param was nullptr ) ( 0 = success, target is still alive ) ( 1 = success, target killed )
	 */
	inline int attack(ActorBase* attacker, ActorBase* target)
	{
		if ( attacker != nullptr && target != nullptr ) {
			// damage is a random value between the actor's max damage, and (max damage / 6)
			int dmg = _rng.get(attacker->getMaxDamage(), (attacker->getMaxDamage() / 6));
			// if actor has enough stamina
			if ( attacker->getStamina() >= _ruleset._attack_cost_stamina )
				target->modHealth(-dmg);
			// if actor has stamina, but not enough for a full attack
			else if ( attacker->getStamina() )
				target->modHealth(-(dmg / 2));
			// actor is out of stamina
			else {
				target->modHealth(-(dmg / 4));
				attacker->modHealth(-(dmg / 12));
			}
			attacker->modStamina(-_ruleset._attack_cost_stamina);
			// check if target died, and return
			if ( target->isDead() ) {
				attacker->addKill();
				return 1;
			}
			else return 0;
		}
		else return -1;
	}

public:
	/** CONSTRUCTOR **  
	 * Gamespace(GLOBAL&, GameRules&)  
	 * Creates a new gamespace with the given settings.
	 * 
	 * @param settings	- A ref to the global settings structure
	 * @param ruleset	- A ref to the ruleset structure
	 */
	Gamespace(GLOBAL& settings, GameRules &ruleset) : _ruleset(ruleset), _world((settings._import_filename.size() > 0) ? (Cell{ settings._import_filename, ruleset._walls_always_visible, settings._override_known_tiles }) : (Cell{ settings._cellSize, ruleset._walls_always_visible, settings._override_known_tiles })), _player("Player", Coord(1, 1), ruleset._player_template)
	{
		_world.modVis(true, _player.pos(), _player._visRange); // allow the player to see the area around them
		generateEnemies(ruleset._enemy_count); // create an amount of enemies specified by the ruleset
	//	generateNeutral(10);
	}

	/**
	 * actionHostile()
	 * Iterates the list of enemies and makes them perform a random action.
	 * Hostiles will follow the player when nearby, and run away if their stamina is too low
	 * 
	 * Note that this function does NOT clean up dead enemies, which is done from FrameBuffer::display()
	 */
	void actionHostile()
	{
		for ( auto it = _hostile.begin(); it < _hostile.end(); it++ ) {
			// Check if this enemy is NOT dead:
			if ( !(&*it)->isDead() ) {
				// if this enemy is aggravated:
				if ( (&*it)->isAggro() ) {
					// This enemy moves
					move(&*it, getDirTo((&*it)->pos(), _player.pos(), isAfraid(&*it)));
					// decrease aggression
					(&*it)->decrementAggro();
				}

				// else this enemy is not aggravated:
				else {
					// If the player is within aggravation distance:
					if ( _player.getDist((&*it)->pos()) <= _ruleset._enemy_aggro_distance ) {
						// This enemy becomes aggravated
						(&*it)->modAggro(_ruleset._enemy_aggro_duration);
						// This enemy moves
						move(&*it, getDirTo((&*it)->pos(), _player.pos(), isAfraid(&*it)));
					}
					
					// Else, roll for move check:
					else if (_rng.get(_ruleset._enemy_move_chance, 0) == 0)
						// Move randomly
						move(&*it, intToDir(_rng.get(3, 0)));
				}
			}
		}
	}

	/**
	 * actionPlayer(char)
	 * Moves the player in a given direction, if possible.
	 * 
	 * @param dir	- 'w' for up, 's' for down, 'a' for left, 'd' for right. Anything else is ignored.
	 * @returns int	- ( 0 = player is dead ) ( 1 = normal execution ) ( 2 = all enemies are dead )
	 */
	int actionPlayer(char key)
	{
		// if not dead and move was successful
		if ( !_player.isDead() && move(&_player, key) ) { // player specific post-movement functions
			_world.modVis(true, _player.pos(), _player._visRange); // allow the player to see the area around them
		}
		// 2nd check to cover both possible cases, where player is dead after moving, or if the first if statement failed
		if ( _player.isDead() )
			return 0;
		else if ( _allEnemiesDead )
			return 2;
		return 1;
	}

	/**
	 * apply_passive()
	 * Iterates through all actors and applies passive effects set by the current ruleset.
	 */
	inline void apply_passive()
	{
		// regen player
		regen(&_player);
		level_up(&_player);
		// regen all hostiles
		for ( auto it = _hostile.begin(); it != _hostile.end(); it++ ) {
			level_up(&*it);
			regen(&*it);
		}
	}

	/**
	 * cleanupDead()
	 * Removes dead hostiles from the game.
	 * This function is used by FrameBuffer::display()
	 */
	inline void cleanupDead()
	{
		for ( size_t it = 0; it < _hostile.size(); it++ )
			if ( _hostile.at(it).isDead() )
				_hostile.erase(_hostile.begin() + it);
		if ( _hostile.size() == 0 )
			_allEnemiesDead = true;
	}

	/**
	 * getActorAt(Coord, const bool)
	 * Returns a pointer to an actor located at a given tile.
	 *
	 * @param pos			- The target tile
	 * @param findByIndex	- (Default: true) Whether to search the matrix from pos (0,0)=true or (1,1)=false
	 */
	ActorBase* getActorAt(Coord pos, const bool findByIndex = true)
	{
		if ( pos == _player.pos() )
			return &_player; // else:
		for ( auto it = _hostile.begin(); it != _hostile.end(); it++ ) {
			if ( pos == (*it).pos() )
				return &*it;
		}
		return{ nullptr };
	}
	ActorBase* getActorAt(int posX, int posY, const bool findByIndex = true)
	{
		if ( posX == _player.pos()._x && posY == _player.pos()._y )
			return &_player; // else:
		for ( auto it = _hostile.begin(); it != _hostile.end(); it++ ) {
			if ( posX == (*it).pos()._x && posY == (*it).pos()._y )
				return &*it;
		}
		return{ nullptr };
	}

	// GETTERS & FRAME API

	inline Player& getPlayer() { return _player; }
	inline Tile& getTile(Coord pos) { return _world.get(pos); }
	inline Tile& getTile(int x, int y) { return _world.get(x, y); }
	inline Cell& getCell() { return{ _world }; }
	inline Coord getCellSize() { return{ _world._max._x, _world._max._y }; }
	inline std::vector<Enemy>& getHostileVec() { return _hostile; }

	// DISPLAYS / HUD
};