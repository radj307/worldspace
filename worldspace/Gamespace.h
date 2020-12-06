/**
 * game.h
 * Represents the game, with a worldspace & actors.
 * Contains the Gamespace class, which is a container for cells, and actors.
 * by radj307
 */
#pragma once
#include "actor.h"
#include "cell.h"
#include "Flare.h"
#include "GameRules.h"
#include "item.h"

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

	bool // Game State Flags
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
	Coord findValidSpawn(const bool isPlayer = false, const bool checkForItems = true)
	{
		// calculate max possible valid positions, set it as max
		const int MAX_CHECKS{ (_world._max._x - 2) * (_world._max._y - 2) };
		// loop
		for ( auto i{ 0 }; i < MAX_CHECKS; i++ ) {
			Coord pos{ 0, 0 };
			for ( auto findPos{ pos }; !_world.get(findPos)._canSpawn; pos = findPos ) {
				findPos = { _rng.get(_world._max._x - 2, 1), _rng.get(_world._max._y - 2, 1) };
			}
			// Check if this pos is valid
			if ( !checkForItems ? true : getItemAt(pos) == nullptr && isPlayer ? true : getActorAt(pos) == nullptr && getDist(_player.pos(), pos) >= _ruleset._enemy_aggro_distance + _player.getVis() ) return pos;
		}
		// Else return invalid coord
		return { -1, -1 };
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
		for ( auto i{ 0 }; i < count; i++ ) {
			unsigned int sel{ 0 };

			for ( auto it{ static_cast<signed>(templates.size() - 1) }; it >= 0; it-- ) {
				if ( _rng.get(100u, 0u) < templates.at(it)._chance ) {
					sel = it;
					break;
				}
			}
			v.push_back({ findValidSpawn(), templates.at(sel) });
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
		for ( auto i{ 0 }; i < count; i++ ) {
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
		for ( auto& it : _hostile ) (this->*func)(&it); // Apply to enemies
		for ( auto& it : _neutral ) (this->*func)(&it); // Apply to neutrals
	}

	/**
	 * apply_to_npc(void(Gamespace::*)(ActorBase*))
	 * @brief Applies a Gamespace function to all NPC actors. Function must: Return void, and take 1 parameter of type ActorBase*
	 *
	 * @param func	- A void function that takes only one ActorBase* parameter.
	 */
	void apply_to_npc(void (Gamespace::*func)(NPC*))
	{
		for ( auto& it : _hostile ) (this->*func)(&it); // Apply to enemies
		for ( auto& it : _neutral ) (this->*func)(&it); // Apply to neutrals
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
	 * regen(ActorBase*)
	 * @brief Increases an actors stats by the relevant value in ruleset.
	 *
	 * @param actor	- Pointer to an actor
	 */
	// ReSharper disable once CppMemberFunctionMayBeConst
	void regen(ActorBase* actor)
	{
		if ( actor != nullptr && !actor->isDead() ) {
			// If actor is not the player, regen double
			if ( actor->faction() != FACTION::PLAYER ) {
				actor->modHealth(_ruleset._regen_health * 2);
				actor->modStamina(_ruleset._regen_stamina * 2);
			}
			else {
				actor->modHealth(_ruleset._regen_health);
				actor->modStamina(_ruleset._regen_stamina);
			}
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
		if ( a != nullptr && _ruleset.canLevelUp(a) ) {
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
	 * getRandomDir()
	 * @brief Returns a random direction char
	 * @returns char	- w/a/s/d
	 */
	char getRandomDir()
	{
		return intToDir(_rng.get(3, 0));
	}

	/**
	 * canMove(Coord)
	 * @brief Returns true if the target position can be moved to, and there is not an actor currently occupying it.
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
	 * @brief Returns true if the target position can be moved to, and there is not an actor currently occupying it.
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
	 * canMove(Coord&, NPC*)
	 * @brief Variant of the canMove() function designed for NPCs. Checks for NPCs of the same faction at the target location.
	 *
	 * @param pos	 - Target position
	 * @param myFac	 - The actor who wants to move to target's faction
	 * @returns bool - ( false = cannot move ) ( true = can move )
	 */
	bool canMove(const Coord& pos, const FACTION myFac)
	{
		if ( _world.get(pos)._canMove ) {
			// check pos for an actor
			auto* target{ getActorAt(pos) };
			// if there is no target, or if there is a target not of my faction
			if ( target == nullptr || (target != nullptr && myFac != target->faction()) )
				return true; // can move to this tile
		} // else
		return false; // cannot move to this tile
	}

	/**
	 * move(ActorBase*, char)
	 * @brief Attempts to move the target actor to an adjacent tile, and processes trap & item logic.
	 *
	 * @param actor	 - A pointer to the target actor
	 * @param dir	 - (w = up / s = down / a = left / d = right) all other characters are ignored.
	 * @returns bool - ( true = moved successfully ) ( false = did not move )
	 */
	bool move(ActorBase* actor, const char dir)
	{
		auto did_move{ false };
		if ( actor != nullptr ) {
			auto* target{ getActorAt(actor->getPosDir(dir)) }; // declare a pointer to potential attack target

			// If the actor killed someone with an attack, move them to the target tile.
			if ( target != nullptr && (attack(actor, target) == 1 && canMove(target->pos())) ) {  // NOLINT(bugprone-branch-clone)
				actor->moveDir(dir);
				did_move = true;
			}
			// Else move randomly
			else if ( canMove(actor->getPosDir(dir)) ) {
				actor->moveDir(dir);
				did_move = true;
			}

			// Check for items
			auto* item{ getItemAt(actor->pos()) };
			if ( item != nullptr ) {
				item->attempt_use(actor);
			}

			// Calculate trap damage if applicable
			if ( _world.get(actor->pos())._isTrap ) {
				if ( _ruleset._trap_percentage ) actor->modHealth(-static_cast<int>(static_cast<float>(actor->getMaxHealth()) * (static_cast<float>(_ruleset._trap_dmg) / 100.0f)));
				else actor->modHealth(-_ruleset._trap_dmg);
			}
		}
		return did_move;
	}

	/**
	 * attack(ActorBase*, ActorBase*)
	 * @brief Allows an actor to attack another actor.
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
		if ( attacker != nullptr && target != nullptr && attacker->faction() != target->faction() ) {
			// damage is a random value between the actor's max damage, and (max damage / 6)
			const auto dmg = _rng.get(attacker->getMaxDamage(), attacker->getMaxDamage() / 6);
			// FULL-ATTACK (actor has enough stamina) - Can't block or parry
			if ( attacker->getStamina() >= _ruleset._attack_cost_stamina ) {
				attacker->modStamina(-_ruleset._attack_cost_stamina);
				target->modHealth(-dmg);
			}
			// HALF-ATTACK (actor has stamina but not enough for a full attack) - Block
			else if ( attacker->getStamina() >= _ruleset._attack_cost_stamina / 3 ) {
				// remove stamina from attacker
				attacker->modStamina(-_ruleset._attack_cost_stamina);
				// check if the target has enough stamina to block
				if ( target->getStamina() >= _ruleset._attack_cost_stamina / 2 ) {
					target->modStamina(-_ruleset._attack_cost_stamina / 2);
					target->modHealth(-(dmg / 20));
				}
				else target->modHealth(-(dmg / 2));
			}
			// MINIMAL-ATTACK (actor does not have any stamina) - Parry
			else { // remove stamina from attacker
				attacker->modStamina(-_ruleset._attack_cost_stamina);
				// check if the target has enough stamina to parry
				if ( target->getStamina() >= _ruleset._attack_cost_stamina / 2 ) {
					// remove stamina from target
					target->modStamina(-(_ruleset._attack_cost_stamina / 2));
					// attacker takes the entire damage value
					attacker->modHealth(-dmg);
				}
				else { // target cannot parry, takes partial damage
					target->modHealth(-(dmg / 4));
					attacker->modHealth(-(dmg / 12));
				}
			}
			// POST-ATTACK CHECKS:
			// if target died
			if ( target->isDead() )
				attacker->addKill(target->getLevel() > attacker->getLevel() ? target->getLevel() - attacker->getLevel() : 1);
			// if attacker died from parry
			else if ( attacker->isDead() )
				target->addKill(attacker->getLevel() > target->getLevel() ? attacker->getLevel() - target->getLevel() : 1);
			else target->setRelationship(attacker->faction(), true);
			return target->isDead();
		}
		return -1;
	}

	/**
	 * moveNPC(NPC*, bool)
	 * @brief Attempt to move an npc with obstacle avoidance towards its current target.
	 *
	 * @param npc	 - Pointer to an NPC instance
	 * @param noFear - NPC will never run away
	 */
	bool moveNPC(NPC* npc, const bool noFear = false)
	{
		auto dir{ npc->getDirTo(noFear) };
		const auto dirAsInt{ dirToInt(dir) };
		// if NPC can move in their chosen direction, return result of move
		if ( canMove(npc->getPosDir(dir), npc->faction()) )
			return move(&*npc, dir);
		// else find a new direction
		switch ( _rng.get(1, 0) ) { // randomly choose order
		case 0: // check adjacent tiles clockwise
			for ( auto it{ dirAsInt - 1 }; it <= dirAsInt + 1; it += 2 ) {
				// check if iterator is a valid direction int
				if ( it >= 0 && it <= 3 ) {
					dir = intToDir(it);
				}
				// check if iterator went below 0, and correct it
				else if ( it == -1 ) {
					dir = intToDir(3);
				}
				// check if iterator went above 3, and correct it
				else if ( it == 4 ) {
					dir = intToDir(0);
				}
				else continue; // undefined
				if ( canMove(npc->getPosDir(dir), npc->faction()) )
					return move(&*npc, dir);
			}
			break;
		case 1: // check adjacent tiles counter-clockwise
			for ( auto it{ dirAsInt + 1 }; it >= dirAsInt - 1; it -= 2 ) {
				// check if iterator is a valid direction int
				if ( it >= 0 && it <= 3 ) {
					dir = intToDir(it);
				}
				// check if iterator went below 0, and correct it
				else if ( it == -1 ) {
					dir = intToDir(3);
				}
				// check if iterator went above 3, and correct it
				else if ( it == 4 ) {
					dir = intToDir(0);
				}
				else continue;
				if ( canMove(npc->getPosDir(dir), npc->faction()) )
					return move(&*npc, dir);
			}
			break;
		default:break;
		}
		// failed, return false
		return false;
	}

	/**
	 * actionNPC(NPC*)
	 * @brief Performs an action for a single NPC
	 *
	 * @param npc	- Pointer to an NPC instance
	 */
	void actionNPC(NPC* npc)   /// SWITCH THIS FUNCTION TO USE NEW NPC FUNCTIONS: [hasTarget(), canSee()]
	{
		// Finale Challenge Event - enemies always attack player, neutrals attack player if ruleset allows it
		if ( _final_challenge && (npc->faction() == FACTION::ENEMY || npc->faction() == FACTION::NEUTRAL && _ruleset._challenge_neutral_is_hostile) ) {
			if ( !npc->isAggro() ) {
				npc->maxAggro();
				npc->setTarget(&_player);
			}
			moveNPC(&*npc, true);
		}
		else { // Normal turn
			// npc is aggravated
			if ( npc->isAggro() && _rng.get(_ruleset._npc_move_chance_aggro, 0) != 0 ) {
				if ( npc->hasTarget() )
					moveNPC(&*npc);
				npc->decrementAggro();
			}
			// npc can see player and is hostile to player
			else if ( npc->canSeeHostile(&_player) ) {
				npc->setTarget(&_player);
				npc->maxAggro();
				moveNPC(&*npc);
			}
			// npc is idle, check nearby
			else {
				auto* const nearest{ getNearbyActor(npc->pos(), npc->getVis()) };
				if ( nearest != nullptr ) {
					if ( npc->canSeeHostile(&*nearest) ) {
						npc->setTarget(&*nearest);
						npc->maxAggro();
						moveNPC(&*npc);
					}
					else if ( _rng.get(_ruleset._npc_move_chance, 0) == 0 )
						move(&*npc, getRandomDir());
				}
				else if ( _rng.get(_ruleset._npc_move_chance, 0) == 0 )
					move(&*npc, getRandomDir());
			}
		}
	}

	/** CONSTEXPR **
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
	explicit Gamespace(GameRules& ruleset) : _world(!ruleset._world_import_file.empty() ? Cell{ ruleset._world_import_file, ruleset._walls_always_visible, ruleset._override_known_tiles } : Cell{ ruleset._cellSize, ruleset._walls_always_visible, ruleset._override_known_tiles }), _ruleset(ruleset), _player({ findValidSpawn(true), ruleset._player_template }), _FLARE_DEF_CHALLENGE(_world._max), _flare(nullptr)
	{
		_hostile = (generate_NPCs<Enemy>(ruleset._enemy_count, ruleset._enemy_template));
		_neutral = (generate_NPCs<Neutral>(ruleset._neutral_count, ruleset._neutral_template));
		_item_static_health = generate_items<ItemStaticHealth>(10, true);
		_item_static_stamina = generate_items<ItemStaticStamina>(10);
		_world.modVisCircle(true, _player.pos(), _player.getVis() + 2); // allow the player to see the area around them

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
		if ( !_player.isDead() && move(&_player, key) ) {
			// player specific post-movement functions
			_world.modVisCircle(true, _player.pos(), _player.getVis() + 2); // allow the player to see the area around them
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
		for ( auto it{ static_cast<int>(_hostile.size()) - 1 }; it >= 0; it-- )
			if ( _hostile.at(it).isDead() ) _hostile.erase(_hostile.begin() + it);
		// erase dead neutrals
		for ( auto it{ static_cast<signed>(_neutral.size() - 1) }; it >= 0; it-- )
			if ( _neutral.at(it).isDead() ) _neutral.erase(_neutral.begin() + it);
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
		for ( auto y{ pos._y - visRange }; y < pos._y + visRange; ++y ) {
			for ( auto x{ pos._x - visRange }; x < pos._x + visRange; ++x ) {
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
		for ( auto* it : get_all_actors() ) {
			if ( pos == it->pos() ) return it;
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
		for ( auto* it : get_all_static_items() ) {
			if ( pos == it->pos() ) return it;
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
	Cell& getCell() { return { _world }; }
	std::vector<Enemy>& getHostileVec() { return _hostile; }
	[[nodiscard]] Coord getCellSize() const { return { _world._max._x, _world._max._y }; }
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