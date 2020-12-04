/**
 * game.h
 * Represents the game, with a worldspace & actors.
 * Contains the Gamespace class, which is a container for cells, and actors.
 * by radj307
 */
#pragma once
#include "actor.h"
#include "cell.h"
#include "GameRules.h"
#include "item.h"

/**
 * class Gamespace
 * Contains all of the game-related functions required for running a game. Does not contain any display functions, use external FrameBuffer.
 */
class Gamespace {
	// worldspace cell
	Cell _world;
	// Reference to the game's ruleset
	GameRules& _ruleset;
	// Randomization engine
	tRand _rng;
	// Functor for checking distance between 2 points
	checkDistance getDist;

	// player character
	Player _player;
	// generic enemies
	std::vector<Enemy> _hostile;
	// neutral actors
	std::vector<Neutral> _neutral;
	// Static Items - Health
	std::vector<ItemStaticHealth> _item_static_health;
	// Static Items - Stamina
	std::vector<ItemStaticStamina> _item_static_stamina;

	/**
	 * findValidSpawn()
	 * Returns the coordinate of a valid NPC spawn position. The player must already be initialized.
	 *
	 * @returns Coord
	 */
	Coord findValidSpawn(const bool isPlayer = false)
	{
		// calculate max possible valid positions, set it as max
		const int MAX_CHECKS{(_world._max._x - 2) * (_world._max._y - 2)};
		// loop
		for (auto i{0}; i < MAX_CHECKS; i++) {
			Coord pos{0,0};
			for ( auto findPos{ pos }; !_world.get(findPos)._canSpawn; pos = findPos ) {
				findPos = { _rng.get(_world._max._x - 2, 1), _rng.get(_world._max._y - 2, 1) };
			}
		//	auto* tile{&_world.get(pos)};
			// Check if this pos is valid
			if (isPlayer ? true : getActorAt(pos) == nullptr && getDist(_player.pos(), pos) >= _ruleset._enemy_aggro_distance + _player.getVis()) return pos;
		}
		// Else return invalid coord
		return Coord(-1, -1);
	}

	template <typename Actor>
	std::vector<Actor> generate_NPCs(const int count, std::vector<ActorTemplate>& templates)
	{
		std::vector<Actor> v;
		for (auto i{0}; i < count; i++) {
			unsigned int sel{0};

			for (auto it{static_cast<signed>(templates.size() - 1)}; it >= 0; it--) {
				if ( _rng.get(100u, 0u) <= templates.at(it)._chance) {
					sel = it;
					break;
				}
			}
			
			v.push_back({findValidSpawn(), templates.at(sel)});
		}
		return v;
	}

	template<typename Item>
	std::vector<Item> generate_items(const int count, const bool lockToPlayer = false)
	{
		std::vector<Item> v;
		for ( auto i{0}; i < count; i++ ) {
			if ( lockToPlayer )
				v.emplace_back(Item{ findValidSpawn(), 50, { FACTION::PLAYER } });
			else
				v.emplace_back(Item{ findValidSpawn(), 50 });
		}
		return v;
	}
	
	/**
	 * apply_to_all(void(Gamespace::*)(ActorBase*))
	 * Applies a Gamespace function to all actors.
	 * Function must: Return void, and take 1 parameter of type ActorBase*
	 *
	 * @param func	- A void function that takes only one ActorBase* parameter.
	 */
	void apply_to_all(void (Gamespace::*func)(ActorBase*))
	{
		// Apply to player
		(this->*func)(&_player);
		// Apply to enemies
		for (auto it{_hostile.begin()}; it != _hostile.end(); ++it) (this->*func)(&*it);
		// Apply to neutrals
		for (auto it{_neutral.begin()}; it != _neutral.end(); ++it) (this->*func)(&*it);
	}

	/**
	 * apply_to_npc(void(Gamespace::*)(ActorBase*))
	 * Applies a Gamespace function to all NPC actors.
	 * Function must: Return void, and take 1 parameter of type ActorBase*
	 *
	 * @param func	- A void function that takes only one ActorBase* parameter.
	 */
	void apply_to_npc(void (Gamespace::*func)(NPC*))
	{
		// Apply to enemies
		for (auto it{_hostile.begin()}; it != _hostile.end(); ++it) (this->*func)(&*it);
		// Apply to neutrals
		for (auto it{_neutral.begin()}; it != _neutral.end(); ++it) (this->*func)(&*it);
	}

	/**
	 * get_all_actors()
	 * Returns a vector of pointers containing all actors in the game.
	 *
	 * @returns vector<ActorBase*>
	 */
	std::vector<ActorBase*> get_all_actors()
	{
		std::vector<ActorBase*> allActors;
		allActors.push_back(&_player);
		for (auto it{_hostile.begin()}; it != _hostile.end(); ++it) allActors.emplace_back(&*it);
		for (auto it{_neutral.begin()}; it != _neutral.end(); ++it) allActors.emplace_back(&*it);
		return allActors;
	}
	/**
	 * get_all_static_items()
	 * Returns a vector of pointers containing all actors in the game.
	 *
	 * @returns vector<ItemStaticBase*>
	 */
	std::vector<ItemStaticBase*> get_all_static_items()
	{
		std::vector<ItemStaticBase*> allItems;
		for ( auto it{ _item_static_health.begin() }; it != _item_static_health.end(); ++it ) allItems.emplace_back(&*it);
		for ( auto it{ _item_static_stamina.begin() }; it != _item_static_stamina.end(); ++it ) allItems.emplace_back(&*it);
		return allItems;
	}

	/**
	 * get_all_npc()
	 * Returns a vector of pointers containing all NPC actors in the game.
	 *
	 * @returns vector<NPC*>
	 */
	std::vector<NPC*> get_all_npc()
	{
		std::vector<NPC*> allActors;
		for (auto it{_hostile.begin()}; it != _hostile.end(); ++it) allActors.push_back(&*it);
		for (auto it{_neutral.begin()}; it != _neutral.end(); ++it) allActors.push_back(&*it);
		return allActors;
	}

	/**
	 * isAfraid(ActorBase*)
	 * Returns true if the given enemy's stats are too low to proceed with an attack on the player
	 *
	 * @param a		 - Pointer to an Enemy instance
	 * @returns bool - ( true = Enemy is afraid, run away ) ( false = Enemy is not afraid, proceed with attack )
	 */
	bool isAfraid(ActorBase* a) const
	{
		if (a != nullptr && (a->getHealth() < a->getMaxHealth() / 5 || a->getStamina() < _ruleset._attack_cost_stamina)) return true;
		return false;
	}

	/**
	 * regen(ActorBase*)
	 * Increases an actors stats by the relevant value in ruleset.
	 *
	 * @param actor	- Pointer to an actor
	 */
	// ReSharper disable once CppMemberFunctionMayBeConst
	void regen(ActorBase* actor)
	{
		if ( actor != nullptr && !actor->isDead()) {
			actor->modHealth(_ruleset._regen_health);
			actor->modStamina(_ruleset._regen_stamina);
		}
	}

	/**
	 * regen(ActorBase*, int)
	 * Increases an actors stats by a percentage.
	 *
	 * @param actor   - Pointer to an actor
	 * @param percent - Percentage to restore ( 0 - 100 )
	 */
	static void regen(ActorBase* actor, int percent)
	{
		if ( actor != nullptr && !actor->isDead() ) {
			// Minimum percentage is 0
			if ( percent < 0 )
				percent = 0;
			// Maximum percentage is 100
			else if ( percent > 100 )
				percent = 100;
			// Modify health+stamina by percentage
			actor->modHealth(percent * actor->getMaxHealth() / 100);
			actor->modStamina(percent * actor->getMaxStamina() / 100);
		}
	}

	/**
	 * level_up(ActorBase*)
	 * Checks if the given actor has enough kills to level up, then increments their level if they do.
	 *
	 * @param a	- Pointer to a target actor
	 */
	void level_up(ActorBase* a)
	{
		// return if nullptr was passed
		if (a != nullptr && _ruleset.canLevelUp(a)) {
			a->addLevel();
			// If actor who leveled up is the player
			if ( a->faction() == FACTION::PLAYER ) {
				regen(&*a, _ruleset._level_up_restore_percent);
				_flare = _ruleset._level_up_flare_time;
			}
		}
	}

	/**
	 * intToDir(int)
	 * Converts an integer to a direction char. Used by NPCs to select a movement direction consistent with the player controls.
	 *
	 * @param i			- ( 0 = 'w'/up ) ( 1 = 's'/down ) ( 2 = 'a'/left ) ( 3 = 'd'/right ) ( other = ' ' )
	 * @returns char	- w/a/s/d
	 */
	static char intToDir(const int i)
	{
		switch (i) {
		case 0: return 'w';
		case 1: return 's';
		case 2: return 'a';
		case 3: return 'd';
		default: return ' ';
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
	char getDirTo(const Coord& pos, const Coord& target, const bool invert = false)
	{
		auto distX{ pos._x - target._x }, distY{ pos._y - target._y };
		// reduce large X distances to -1 or 1
		if ( distX < -1 )		distX = -1;
		else if ( distX > 1 )	distX = 1;
		// reduce large Y distances to -1 or 1
		if ( distY < -1 )		distY = -1;
		else if ( distY > 1 )	distY = 1;

		if ( _rng.get(100, 0) >= 50 ) { // Check X-axis first
			// select a direction
			if ( distX == 0 && !invert ) return distY < 0 ? 's' : 'w'; // check if X-axis is aligned
			if ( distY == 0 && !invert ) return distX < 0 ? 'd' : 'a'; // check if Y-axis is aligned
			if ( distX == 0 && invert )	 return distY < 0 ? 'w' : 's'; // check if X-axis is aligned (inverted)
			if ( distY == 0 && invert )	 return distX < 0 ? 'a' : 'd'; // check if Y-axis is aligned (inverted)
			// neither axis is aligned, select a random direction
			switch ( _rng.get(1, 0) ) {
			case 1: // move on Y-axis
				if ( !invert )
					return distY < 0 ? 's' : 'w';
				return distY < 0 ? 'w' : 's';
			case 0: // move on X-axis
				if ( !invert )
					return distX < 0 ? 'd' : 'a';
				return distX < 0 ? 'a' : 'd';
			default:return' ';
			}
		}
		// Check Y-axis first
		// select a direction
		if ( distY == 0 && !invert ) return distX < 0 ? 'd' : 'a'; // check if Y-axis is aligned
		if ( distX == 0 && !invert ) return distY < 0 ? 's' : 'w'; // check if X-axis is aligned
		if ( distY == 0 && invert )	 return distX < 0 ? 'a' : 'd'; // check if Y-axis is aligned (inverted)
		if ( distX == 0 && invert )	 return distY < 0 ? 'w' : 's'; // check if X-axis is aligned (inverted)
		// neither axis is aligned, select a random direction
		switch ( _rng.get(1, 0) ) {
		case 0: // move on Y-axis
			if ( !invert )
				return distY < 0 ? 's' : 'w';
			return distY < 0 ? 'w' : 's';
		case 1: // move on X-axis
			if ( !invert )
				return distX < 0 ? 'd' : 'a';
			return distX < 0 ? 'a' : 'd';
		default:return' ';
		}
	}

	/**
	 * canMove(Coord)
	 * Returns true if the target position can be moved to.
	 *
	 * @param pos	 - Target position
	 * @returns bool - ( false = cannot move ) ( true = can move )
	 */
	bool canMove(const Coord& pos)
	{
		return _world.get(pos)._canMove && getActorAt(pos) == nullptr ? true : false;
	}

	/**
	 * canMove(int, int)
	 * Returns true if the target position can be moved to.
	 *
	 * @param posX	 - Target X position
	 * @param posY	 - Target Y position
	 * @returns bool - ( false = cannot move ) ( true = can move )
	 */
	bool canMove(const int posX, const int posY)
	{
		return _world.get(posX, posY)._canMove && getActorAt(posX, posY) == nullptr ? true : false;
	}

	/**
	 * move(ActorBase*, char)
	 * Attempts to move the target actor to an adjacent tile, and processes trap logic.
	 *
	 * @param actor	 - A pointer to the target actor
	 * @param dir	 - (w = up / s = down / a = left / d = right) all other characters are ignored.
	 * @returns bool - ( true = moved successfully ) ( false = did not move )
	 */
	bool move(ActorBase* actor, const char dir)
	{
		auto did_move{false};
		if (actor != nullptr) {
			auto* target{getActorAt(actor->getPosDir(dir))}; // declare a pointer to potential attack target

			// If the actor killed someone with an attack, move them to the target tile.
			if (target != nullptr && (attack(actor, target) == 1 && canMove(target->pos()))) {  // NOLINT(bugprone-branch-clone)
				actor->moveDir(dir);
				did_move = true;
			}
			// Else move randomly
			else if (canMove(actor->getPosDir(dir))) {
				actor->moveDir(dir);
				did_move = true;
			}
			// Check for items
			auto* item{ getItemAt(actor->pos()) };
			if ( item != nullptr ) {
				item->attempt_use(actor);
			}
			// Calculate trap damage if applicable
			if (_world.get(actor->pos())._isTrap) {
				if (_ruleset._trap_percentage) actor->modHealth(-static_cast<int>(static_cast<float>(actor->getMaxHealth()) * (static_cast<float>(_ruleset._trap_dmg) / 100.0f)));
				else actor->modHealth(-_ruleset._trap_dmg);
			}
		}
		return did_move;
	}
	
	/**
	 * attack(ActorBase*, ActorBase*)
	 * Allows an actor to attack another actor.
	 *
	 * @param attacker	- A pointer to the attacking actor
	 * @param target	- A pointer to the actor being attacked
	 * @returns int		- ( -1 = attack failed ) ( 0 = success, target is still alive ) ( 1 = success, target killed )
	 */
	int attack(ActorBase* attacker, ActorBase* target)
	{
		// return early if target is player and godmode is enabled
		if ( _ruleset._player_godmode && target->faction() == FACTION::PLAYER )
			return -1;
		// Calculate attack
		if (attacker != nullptr && target != nullptr && attacker->faction() != target->faction()) {
			// damage is a random value between the actor's max damage, and (max damage / 6)
			const auto dmg = _rng.get(attacker->getMaxDamage(), attacker->getMaxDamage() / 6);
			// if actor has enough stamina
			if (attacker->getStamina() >= _ruleset._attack_cost_stamina) 
				target->modHealth(-dmg);
			// if actor has stamina, but not enough for a full attack
			else if (attacker->getStamina()) 
				target->modHealth(-(dmg / 2));
			// actor is out of stamina
			else {
				target->modHealth(-(dmg / 4));
				attacker->modHealth(-(dmg / 12));
			}
			attacker->modStamina(-_ruleset._attack_cost_stamina);
			// check if target died, add a kill to attacker
			if (target->isDead()) {
				if ( target->faction() == FACTION::ENEMY ) { // add the difference in level to kill count
					attacker->addKill(target->getLevel() > attacker->getLevel() ? target->getLevel() - attacker->getLevel() : 1);
				}
				else attacker->addKill();
			}
			// else set target as hostile to attacker
			else target->setRelationship(attacker->faction(), true);
			return target->isDead();
		}
		return -1;
	}

	/**
	 * actionNPC(NPC*)
	 * Performs an action for a single NPC
	 *
	 * @param npc	- Pointer to an NPC instance
	 */
	void actionNPC(NPC* npc)
	{
		if (npc->isAggro()) {
			auto* const target{npc->getTarget()};
			if (target != nullptr) move(&*npc, getDirTo(npc->pos(), target->pos(), isAfraid(&*npc)));
			npc->decrementAggro();
		}
		else if ( npc->isHostileTo(FACTION::PLAYER) && _player.getDist(npc->pos()) <= npc->getVis() ) {
			npc->setTarget(&_player);
			npc->maxAggro();
			move(&*npc, getDirTo(npc->pos(), _player.pos(), isAfraid(&*npc)));
		}
		else {
			auto* const nearest{ getNearbyActors(npc->pos(), npc->getVis()) };
			if ( nearest != nullptr ) {
				if ( npc->isHostileTo(nearest->faction()) && getDist(npc->pos(), nearest->pos()) <= npc->getVis() ) {
					npc->setTarget(&*nearest);
					npc->maxAggro();
					move(&*npc, getDirTo(npc->pos(), nearest->pos(), isAfraid(&*npc)));
				}
				else if ( _rng.get(_ruleset._enemy_move_chance, 0) == 0 ) move(&*npc, intToDir(_rng.get(3, 0)));
			}
			else if ( _rng.get(_ruleset._enemy_move_chance, 0) == 0 ) move(&*npc, intToDir(_rng.get(3, 0)));
		}
	}

public:
	// THESE VARIABLES SHOULD BE ACCESSED BY THE FRAMEBUFFER ONLY
	// flare should always be set to a multiple of 2, or 0 for disabled. Flare will flash the display to indicate a level up.
	int _flare {0};
	unsigned short _flare_color{ BACKGROUND_GREEN };

	// THESE VARIABLES SHOULD BE ACCESSED BY THE THREADS & GAMESPACE ONLY
	// When this is true, the player wins.
	bool _allEnemiesDead{ false }, _playerDead{ false };

	/** CONSTRUCTOR **
	 * Gamespace(GLOBAL&, GameRules&)
	 * Creates a new gamespace with the given settings.
	 *
	 * @param ruleset	- A ref to the ruleset structure
	 */
	explicit Gamespace(GameRules& ruleset)
		: _world(!ruleset._world_import_file.empty() ? Cell{ ruleset._world_import_file, ruleset._walls_always_visible, ruleset._override_known_tiles} : Cell{ruleset._cellSize, ruleset._walls_always_visible, ruleset._override_known_tiles}), _ruleset(ruleset), _player({ findValidSpawn(true), ruleset._player_template })
	{
		_hostile = (generate_NPCs<Enemy>(ruleset._enemy_count, ruleset._enemy_template));
		_neutral = (generate_NPCs<Neutral>(ruleset._neutral_count, ruleset._neutral_template));
		_item_static_health = generate_items<ItemStaticHealth>(10, true);
		_item_static_stamina = generate_items<ItemStaticStamina>(10);
		_world.modVis(true, _player.pos(), _player.getVis()); // allow the player to see the area around them
	}

	/**
	 * actionAllNPC()
	 * Performs an action for all NPC instances
	 */
	void actionAllNPC()
	{
		apply_to_npc(&Gamespace::actionNPC);
	}

	/**
	 * actionPlayer(char)
	 * Moves the player in a given direction, if possible.
	 *
	 * @param key	- 'w' for up, 's' for down, 'a' for left, 'd' for right. Anything else is ignored.
	 */
	void actionPlayer(const char key)
	{
		// if not dead and move was successful
		if (!_player.isDead() && move(&_player, key)) {
			// player specific post-movement functions
			_world.modVis(true, _player.pos(), _player.getVis()); // allow the player to see the area around them
		}
	}

	void apply_level_ups()
	{
		apply_to_all(&Gamespace::level_up);
	}

	/**
	 * apply_passive()
	 * Applies passive regen effects to all actors
	 */
	void apply_passive()
	{
		apply_to_all(&Gamespace::regen);
	}

	/**
	 * cleanupDead()
	 * Removes dead hostiles from the game.
	 * This function is used by FrameBuffer::display()
	 */
	void cleanupDead()
	{
		// Erase used health potions
		for ( auto it{ static_cast<signed>(_item_static_health.size() - 1) }; it >= 0; it-- )
			if ( _item_static_health.at(it).getUses() <= 0 )
				_item_static_health.erase(_item_static_health.begin() + it);
		// Erase used stamina potions
		for ( auto it{ static_cast<signed>(_item_static_stamina.size() - 1) }; it >= 0; it-- )
			if ( _item_static_stamina.at(it).getUses() <= 0 )
				_item_static_stamina.erase(_item_static_stamina.begin() + it);
		// erase dead enemies
		//	for ( size_t it = 0; it < _hostile.size(); it++ )
		for (auto it{static_cast<int>(_hostile.size()) - 1}; it >= 0; it--)
			if (_hostile.at(it).isDead()) _hostile.erase(_hostile.begin() + it);
		// erase dead neutrals
		//for ( size_t it = 0; it < _neutral.size(); it++ )
		for (auto it{static_cast<signed>(_neutral.size() - 1)}; it >= 0; it--)
			if (_neutral.at(it).isDead()) _neutral.erase(_neutral.begin() + it);
		// check win condition
		if (_hostile.empty()) _allEnemiesDead = true;
	}

	ActorBase* getNearbyActors(const Coord pos, const int visRange)
	{
		ActorBase* nearest{ nullptr };
		auto dist{ -1 };		
		for ( auto y{pos._y - visRange}; y < pos._y + visRange; ++y ) {
			for ( auto x{pos._x - visRange}; x < pos._x + visRange; ++x ) {
				auto* const here{ getActorAt(x, y) };
				if ( here != nullptr ) {
					const auto dist_here{ getDist(x, y, here->pos()._x, here->pos()._y) };
					if ( dist == -1 || dist_here < dist ) {
						nearest = here;
						dist = dist_here;
					}
					if ( dist == 1 )
						break;
				}
			}
		}
		return nearest;
	}

	/**
	 * getActorAt(Coord)
	 * Returns a pointer to an actor located at a given tile.
	 *
	 * @param pos			- The target tile
	 */
	ActorBase* getActorAt(const Coord& pos)
	{
		for (auto* it : get_all_actors()) {
			if (pos == it->pos()) return it;
		}
		return nullptr;
	}
	ActorBase* getActorAt(const int posX, const int posY)
	{
		for ( auto* it : get_all_actors() ) {
			if ( posX == it->pos()._x && posY == it->pos()._y ) return it;
		}
		return nullptr;
	}
	
	/**
	 * getItemAt(Coord)
	 * Returns a pointer to an actor located at a given tile.
	 *
	 * @param pos			- The target tile
	 */
	ItemStaticBase* getItemAt(const Coord& pos)
	{
		for (auto* it : get_all_static_items()) {
			if (pos == it->pos()) return it;
		}
		return nullptr;
	}
	ItemStaticBase* getItemAt(const int posX, const int posY)
	{
		for ( auto* it : get_all_static_items() ) {
			if ( posX == it->pos()._x && posY == it->pos()._y ) return it;
		}
		return nullptr;
	}


	// GETTERS & FRAME API

	Player& getPlayer() { return _player; }
	Tile& getTile(const Coord& pos) { return _world.get(pos); }
	Tile& getTile(int x, int y) { return _world.get(x, y); }
	Cell& getCell() { return {_world}; }
	[[nodiscard]] Coord getCellSize() const { return {_world._max._x, _world._max._y}; }
	std::vector<Enemy>& getHostileVec() { return _hostile; }
	[[nodiscard]] GameRules& getRuleset() const { return _ruleset; }

	// DISPLAYS / HUD
};