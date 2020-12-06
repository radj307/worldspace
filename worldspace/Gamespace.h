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
#include "Flare.h"

/**
 * class Gamespace
 * @brief Contains all of the game-related functions required for running a game. Does not contain any display functions, use external FrameBuffer.
 */
class Gamespace final {
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
	
	// Declare Flare instances
	FlareLevel _FLARE_DEF_LEVEL;			// Flare used when the player levels up
	FlareChallenge _FLARE_DEF_CHALLENGE;	// Flare used when the final challenge mode begins
	Flare* _flare;	// Pointer to one of the above instances, this should only be accessed through the flare functions.
	
	bool // Flags
		_final_challenge{ false },	// When true, all enemies (& neutrals if set in gamerules) attack the player.
		_allEnemiesDead{ false },	// When true, the player wins
		_playerDead{ false };		// When true, the player loses

	/**
	 * changeFlare(Flare&)
	 * Resets the current flare instance, and applies a new one.
	 * This function should be used rather than directly accessing the _flare pointer.
	 *
	 * @param newFlare	- A new flare instance.
	 */
	void changeFlare(Flare& newFlare)
	{
		if ( _flare != nullptr )
			_flare->reset(); // reset flare instance
		_flare = &newFlare;
	}
	
	/**
	 * findValidSpawn()
	 * @brief Returns the coordinate of a valid NPC spawn position. The player must already be initialized.
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

	/**
	 * generate_NPCs(int, vector<ActorTemplate>)
	 * @brief Returns a vector of randomly generated NPC actors.
	 *
	 * @tparam Actor		 - The type of item to generate
	 * @param count			 - The number of items to generate
	 * @param templates		 - When true, only the player can use these items
	 * @returns vector<Actor>
	 */
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

	/**
	 * generate_items(int, bool)
	 * @brief Returns a vector of randomly generated static items.
	 *
	 * @tparam Item			 - The type of item to generate
	 * @param count			 - The number of items to generate
	 * @param lockToPlayer	 - When true, only the player can use these items
	 */
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
	 * @brief Applies a Gamespace function to all actors. Function must: Return void, and take 1 parameter of type ActorBase*
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
	 * @brief Applies a Gamespace function to all NPC actors. Function must: Return void, and take 1 parameter of type ActorBase*
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
	 * @brief Returns a vector of pointers containing all actors in the game.
	 *
	 * @returns vector<ActorBase*>
	 */
	std::vector<ActorBase*> get_all_actors()
	{
		std::vector<ActorBase*> allActors;
		allActors.reserve(_hostile.size() + _neutral.size() + 1u);
		allActors.emplace_back(&_player);
		for ( auto& it : _hostile ) allActors.emplace_back(&it);
		for ( auto& it : _neutral ) allActors.emplace_back(&it);
		return allActors;
	}
	/**
	 * get_all_static_items()
	 * @brief Returns a vector of pointers containing all actors in the game.
	 *
	 * @returns vector<ItemStaticBase*>
	 */
	std::vector<ItemStaticBase*> get_all_static_items()
	{
		std::vector<ItemStaticBase*> allItems;
		allItems.reserve(_item_static_health.size() + _item_static_stamina.size());
		for ( auto& it : _item_static_health ) allItems.emplace_back(&it);
		for ( auto& it : _item_static_stamina ) allItems.emplace_back(&it);
		return allItems;
	}

	/**
	 * get_all_npc()
	 * @brief Returns a vector of pointers containing all NPC actors in the game.
	 *
	 * @returns vector<NPC*>
	 */
	std::vector<NPC*> get_all_npc()
	{
		std::vector<NPC*> allActors;
		allActors.reserve(_hostile.size() + _neutral.size());
		for ( auto& it : _hostile ) allActors.emplace_back(&it);
		for ( auto& it : _neutral ) allActors.emplace_back(&it);
		return allActors;
	}

	/**
	 * regen(ActorBase*)
	 * @brief Increases an actors stats by the relevant value in ruleset.
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
	 * @brief Increases an actors stats by a percentage.
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
	 * @brief Checks if the given actor has enough kills to level up, then increments their level if they do.
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
				if ( _flare == nullptr )
					changeFlare(_FLARE_DEF_LEVEL);
			}
		}
	}

	/**
	 * intToDir(int)
	 * @brief Converts an integer to a direction char
	 *
	 * @param i		 - Input integer between 0 and 3
	 * @returns char - ( 'w' == 0 ) ( 'd' == 1 ) ( 's' == 2 ) ( 'a' == 3 ) ( ' ' == invalid parameter )
	 */
	static char intToDir(const int i)
	{
		switch ( i ) {
		case 0: return 'w';
		case 1: return 'd';
		case 2: return 's';
		case 3: return 'a';
		default:return ' ';
		}
	}

	/**
	 * intToDir(int)
	 * @brief Converts an integer to a direction char
	 *
	 * @param c		 - Input integer between 0 and 3
	 * @returns int - ( 0 == 'w' ) ( 1 == 'd' ) ( 2 == 's' ) ( 3 == 'a' ) ( -1 == invalid parameter )
	 */
	static int dirToInt(const char c)
	{
		switch ( c ) {
		case 'w': return 0;
		case 'd': return 1;
		case 's': return 2;
		case 'a': return 3;
		default:return -1;
		}
	}
	
	/**
	 * getRandomDir()
	 * @brief Returns a random direction char
	 * @returns char	- w/a/s/d
	 */
	char getRandomDir()
	{
		return intToDir(_rng.get(0, 3));
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
	 * Attempts to move the target actor to an adjacent tile, and processes trap & item logic.
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
		if ( _ruleset._player_godmode && target->faction() == FACTION::PLAYER )	return -1;
		// Calculate attack
		if (attacker != nullptr && target != nullptr && attacker->faction() != target->faction()) {
			// damage is a random value between the actor's max damage, and (max damage / 6)
			const auto dmg = _rng.get(attacker->getMaxDamage(), attacker->getMaxDamage() / 6);
			if (attacker->getStamina() >= _ruleset._attack_cost_stamina) // actor has enough stamina
				target->modHealth(-dmg);
			else if (attacker->getStamina()) // actor has stamina, but not enough for a full attack
				target->modHealth(-(dmg / 2));
			else {							// actor is out of stamina
				target->modHealth(-(dmg / 4));
				attacker->modHealth(-(dmg / 12));
			}
			// Subtract stamina
			attacker->modStamina(-_ruleset._attack_cost_stamina);
			if (target->isDead()) {			// if target died, add a kill to attacker
				if ( target->faction() == FACTION::ENEMY ) // add the difference in level to kill count, but only for enemies
					attacker->addKill(target->getLevel() > attacker->getLevel() ? target->getLevel() - attacker->getLevel() : 1);
				else attacker->addKill();
			}
			// else set target as hostile to attacker
			else target->setRelationship(attacker->faction(), true);
			return target->isDead();
		}
		return -1;
	}

	/**
	 * moveNPC(NPC*, Coord&)
	 * @brief Attempt to move an npc with obstacle avoidance
	 *
	 * @param npc	 - Pointer to an NPC instance
	 * @param target - Ref of a target coordinate
	 * @param noFear - NPC will never run away
	 */
	bool moveNPC(NPC* npc, const Coord& target, const bool noFear = false)
	{
		auto dir{ npc->getDirTo(target, noFear) };
		const auto dirAsInt{ dirToInt(dir) };
		// if NPC can move in their chosen direction, return result of move
		if ( canMove(npc->getPosDir(dir)) )
			return move(&*npc, dir);

		// else find a new direction
		switch ( _rng.get(1u, 0u) ) { // randomly choose order
		case 0: // check adjacent tiles
			for ( auto it{ dirAsInt - 1 }; it <= dirAsInt + 1; it+=2 ) {
				// check if iterator is a valid direction int
				if ( it >= 0 && it <= 3 ) {
					dir = intToDir(it);
					if ( canMove(npc->getPosDir(dir)) )
						return move(&*npc, dir);
				}
				// else continue
			}
			break;
		case 1: // check adjacent tiles
			for ( auto it{ dirAsInt + 1 }; it >= dirAsInt - 1; it-=2 ) {
				// check if iterator is a valid direction int
				if ( it >= 0 && it <= 3 ) {
					dir = intToDir(it);
					if ( canMove(npc->getPosDir(dir)) )
						return move(&*npc, dir);
				}
				// else continue
			}
			break;
		default:break;
		}
		
		// failed, return false
		return false;
	}
	
	/**
	 * actionNPC(NPC*)
	 * Performs an action for a single NPC
	 *
	 * @param npc	- Pointer to an NPC instance
	 */
	void actionNPC(NPC* npc)
	{
		// Finale Challenge Event - enemies always attack player, neutrals attack player if ruleset allows it
		if ( _final_challenge && (npc->faction() == FACTION::ENEMY || npc->faction() == FACTION::NEUTRAL && _ruleset._challenge_neutral_is_hostile) ) {
			if ( !npc->isAggro() ) {
				npc->maxAggro();
				npc->setTarget(&_player);
			}
			moveNPC(&*npc, _player.pos(), true);
	//		move(&*npc, npc->getDirTo(&_player, true));
		}
		else { // Normal turn
			if ( npc->isAggro() ) {
				auto* const target{ npc->getTarget() };
				if ( target != nullptr )
					moveNPC(&*npc, target->pos());
	//				move(&*npc, npc->getDirTo(&*target));
				npc->decrementAggro();
			}
			else if ( npc->isHostileTo(FACTION::PLAYER) && _player.getDist(npc->pos()) <= npc->getVis() ) {
				npc->setTarget(&_player);
				npc->maxAggro();
				moveNPC(&*npc, _player.pos());
	//			move(&*npc, npc->getDirTo(&_player));
			}
			else {
				auto* const nearest{ getNearbyActor(npc->pos(), npc->getVis()) };
				if ( nearest != nullptr ) {
					if ( npc->isHostileTo(nearest->faction()) && getDist(npc->pos(), nearest->pos()) <= npc->getVis() ) {
						npc->setTarget(&*nearest);
						npc->maxAggro();
						moveNPC(&*npc, nearest->pos());
	//					move(&*npc, npc->getDirTo(nearest));
					}
					else if ( _rng.get(_ruleset._enemy_move_chance, 0) == 0 ) move(&*npc, getRandomDir());
				}
				else if ( _rng.get(_ruleset._enemy_move_chance, 0) == 0 ) move(&*npc, getRandomDir());
			}
		}
	}

	/**
	 * trigger_final_challenge(unsigned int)
	 * @brief Checks if the finale event should be triggered
	 *
	 * @param remainingEnemies	- The number of remaining enemies. (_hostile.size())
	 * @returns bool			- ( true = trigger event ) ( false = don't trigger event yet )
	 */
	[[nodiscard]] constexpr bool trigger_final_challenge(const unsigned int remainingEnemies) const { return remainingEnemies <= _ruleset._enemy_count * _ruleset._challenge_final_trigger_percent / 100; }

public:

	/** CONSTRUCTOR **
	 * Gamespace(GLOBAL&, GameRules&)
	 * @brief Creates a new gamespace with the given settings.
	 *
	 * @param ruleset	- A ref to the ruleset structure
	 */
	explicit Gamespace(GameRules& ruleset) : _world(!ruleset._world_import_file.empty() ? Cell{ ruleset._world_import_file, ruleset._walls_always_visible, ruleset._override_known_tiles} : Cell{ruleset._cellSize, ruleset._walls_always_visible, ruleset._override_known_tiles}), _ruleset(ruleset), _player({ findValidSpawn(true), ruleset._player_template }), _FLARE_DEF_CHALLENGE(_world._max), _flare(nullptr)
	{
		_hostile = (generate_NPCs<Enemy>(ruleset._enemy_count, ruleset._enemy_template));
		_neutral = (generate_NPCs<Neutral>(ruleset._neutral_count, ruleset._neutral_template));
		_item_static_health = generate_items<ItemStaticHealth>(10, true);
		_item_static_stamina = generate_items<ItemStaticStamina>(10);
		_world.modVis(true, _player.pos(), _player.getVis()); // allow the player to see the area around them

	#ifdef _DEBUG // debug section
		_final_challenge = true;
		changeFlare(_FLARE_DEF_CHALLENGE);
	#endif
	}

	/**
	 * actionAllNPC()
	 * @brief Performs an action for all NPC instances
	 */
	void actionAllNPC()
	{
		// Check if the final challenge should be triggered
		if ( !_final_challenge && trigger_final_challenge(_hostile.size()) ) {
			_final_challenge = true;
			changeFlare(_FLARE_DEF_CHALLENGE);
		}
		// Perform all NPC actions.
		apply_to_npc(&Gamespace::actionNPC);
	}

	/**
	 * actionPlayer(char)
	 * @brief Moves the player in a given direction, if possible.
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

	/**
	 * apply_level_ups()
	 * @brief Applies pending level ups to all actors.
	 */
	void apply_level_ups() { apply_to_all(&Gamespace::level_up); }

	/**
	 * apply_passive()
	 * @brief Applies passive regen effects to all actors. Amounts are determined by the ruleset.
	 */
	void apply_passive() { apply_to_all(&Gamespace::regen); }

	/**
	 * cleanupDead()
	 * @brief Cleans up expired game elements. This is called by the frame buffer before every frame.
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
		for (auto it{static_cast<int>(_hostile.size()) - 1}; it >= 0; it--)
			if (_hostile.at(it).isDead()) _hostile.erase(_hostile.begin() + it);
		// erase dead neutrals
		for (auto it{static_cast<signed>(_neutral.size() - 1)}; it >= 0; it--)
			if (_neutral.at(it).isDead()) _neutral.erase(_neutral.begin() + it);
		// check win condition
		if ( _player.isDead() ) _playerDead = true;
		if ( _hostile.empty() ) _allEnemiesDead = true;
	}

	/**
	 * getNearbyActor(Coord&, int)
	 * @brief Returns a pointer to the closest actor to a given position.
	 *
	 * @param pos		- Position ref
	 * @param visRange	- Range to check in all directions around the position ref
	 * @returns ActorBase*
	 */
	ActorBase* getNearbyActor(const Coord& pos, const int visRange)
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
	 * @returns ActorBase*	- nullptr if not found
	 */
	ActorBase* getActorAt(const Coord& pos)
	{
		for (auto* it : get_all_actors()) {
			if (pos == it->pos()) return it;
		}
		return nullptr;
	}
	
	/**
	 * getActorAt(int, int)
	 * Returns a pointer to an actor located at a given tile.
	 *
	 * @param posX			- The target tile's X (horizontal) index
	 * @param posY			- The target tile's Y (vertical) index
	 * @returns ActorBase*	- nullptr if not found
	 */
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
	 * @param pos				- The target tile
	 * @returns ItemStaticBase*	- nullptr if not found
	 */
	ItemStaticBase* getItemAt(const Coord& pos)
	{
		for (auto* it : get_all_static_items()) {
			if (pos == it->pos()) return it;
		}
		return nullptr;
	}

	/**
	 * getItemAt(Coord)
	 * Returns a pointer to an item located at a given tile.
	 *
	 * @param posX				- The target tile's X (horizontal) index
	 * @param posY				- The target tile's Y (vertical) index
	 * @returns ItemStaticBase*	- nullptr if not found
	 */
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
	Tile& getTile(const int x, const int y) { return _world.get(x, y); }
	Cell& getCell() { return {_world}; }
	std::vector<Enemy>& getHostileVec() { return _hostile; }
	[[nodiscard]] Coord getCellSize() const { return {_world._max._x, _world._max._y}; }
	[[nodiscard]] GameRules& getRuleset() const { return _ruleset; }
	// Returns true if the player lost the game
	[[nodiscard]] bool playerLost() const { return _playerDead; }
	// Returns true if the player won the game
	[[nodiscard]] bool playerWon() const { return _allEnemiesDead; }
	[[nodiscard]] Flare* getFlare() const { return _flare; }
	void resetFlare()
	{
		_flare->reset(); // reset the flare instance
		_flare = nullptr;// set the flare pointer to nullptr
	}
};