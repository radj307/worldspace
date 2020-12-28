#include "Gamespace.h"
// Gamespace Constructor
#pragma region GAME_CONSTRUCTOR
/** CONSTRUCTOR **
 * Gamespace(GLOBAL&, GameRules&)
 * @brief Creates a new gamespace with the given settings.
 *
 * @param ruleset	 - A ref to the ruleset structure
 */
Gamespace::Gamespace(GameRules& ruleset) : _world(Cell{ ruleset._cellSize, ruleset._walls_always_visible, ruleset._override_known_tiles }), _ruleset(ruleset), _player({ findValidSpawn(true), ruleset._player_template }), _FLARE_DEF_CHALLENGE(_world._max)
{
	_hostile = generate_NPCs<Enemy>(ruleset._enemy_count, ruleset._enemy_template);
	_neutral = generate_NPCs<Neutral>(ruleset._neutral_count, ruleset._neutral_template);
	_item_static_health = generate_items<ItemStaticHealth>(10, true);
	_item_static_stamina = generate_items<ItemStaticStamina>(10);
	_world.modVisCircle(true, _player.pos(), _player.getVis() + 2); // allow the player to see the area around them

#ifdef _DEBUG
//	_game_state._final_challenge.store(true);
//	_flare = &_FLARE_DEF_CHALLENGE;
#endif
}
#pragma endregion		GAME_CONSTRUCTOR
// Gamespace functions related to creating objects in the cell.
#pragma region GAME_SPAWNING
/**
 * findValidSpawn()
 * @brief Returns the coordinate of a valid NPC spawn position. The player must already be initialized.
 *
 * @param isPlayer		- When true, does not check positions for proximity to the player.
 * @param checkForItems	- When true, does not check positions for static items.
 * @returns Coord
 */
Coord Gamespace::findValidSpawn(const bool isPlayer, const bool checkForItems)
{
	// calculate max possible valid positions, set it as max
	const int MAX_CHECKS{ (_world._max._x - 2) * (_world._max._y - 2) };
	// loop
	for ( auto i{ 0 }; i < MAX_CHECKS; i++ ) {
		Coord pos{ 0, 0 };
		for ( auto findPos{ pos }; !_world.canSpawn(findPos); pos = findPos )
			findPos = { _rng.get(_world._max._x - 2, 1), _rng.get(_world._max._y - 2, 1) };
		// Check if this pos is valid
		if ( !checkForItems ? true : getItemAt(pos) == nullptr && isPlayer ? true : getActorAt(pos) == nullptr && getDist(_player.pos(), pos) >= _ruleset._enemy_aggro_distance + _player.getVis() )
			return pos;
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
std::vector<Actor> Gamespace::generate_NPCs(const int count, std::vector<ActorTemplate>& templates)
{
	std::vector<Actor> v;
	for ( auto i{ 0 }; i < count; i++ ) {
		unsigned int sel{ 0 };

		for ( auto it{ static_cast<signed>(templates.size() - 1) }; it >= 0; it-- ) {
			if ( _rng.get(100.0f, 0.0f) < templates.at(it)._chance ) {
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
std::vector<Item> Gamespace::generate_items(const int count, const bool lockToPlayer)
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
 * spawn(ActorTemplate&)
 * @brief Spawns a new NPC mid-game at a random position
 * @tparam NPC			- Type of NPC to build.
 * @param actorTemplate	- Templated stats for the actor.
 * @returns NPC
 */
template<typename NPC> NPC Gamespace::build_npc(ActorTemplate& actorTemplate)
{
	return{ findValidSpawn(), actorTemplate };
}
/**
 * spawn(ActorTemplate&)
 * @brief Spawns a new NPC mid-game at a given position. Throws std::exception if the position is invalid.
 * @tparam NPC			- Type of NPC to build
 * @param pos			- Position of spawned NPC
 * @param actorTemplate	- Templated stats for the actor.
 * @returns NPC
 */
template<typename NPC> NPC Gamespace::build_npc(const Coord& pos, ActorTemplate& actorTemplate)
{
	if ( _world.isValidPos(pos) )
		return{ pos, actorTemplate };
	throw std::exception("Attempted to create an NPC at an invalid position.");
}

void Gamespace::spawn_boss()
{
	_hostile.push_back(build_npc<Enemy>(_ruleset._enemy_boss_template.at(_rng.get(_ruleset._enemy_boss_template.size() - 1, 0u))));
}
#pragma endregion			GAME_SPAWNING
// Gamespace functions that apply other functions to multiple types of objects.
#pragma region GAME_APPLY_TO_TYPE
/**
 * apply_to_all(void(Gamespace::*)(ActorBase*))
 * @brief Applies a Gamespace function to all actors. Function must: Return void, and take 1 parameter of type ActorBase*
 *
 * @param func	- A void function that takes only one ActorBase* parameter.
 */
void Gamespace::apply_to_all(void (Gamespace::*func)(ActorBase*))
{
	// Apply to player
	(this->*func)(&_player);
	for ( auto& it : _hostile ) (this->*func)(&it); // Apply to enemies
	for ( auto& it : _neutral ) (this->*func)(&it); // Apply to neutrals
}

/**
 * apply_to_npc(void(Gamespace::*)(NPC*))
 * @brief Applies a Gamespace function to all NPC actors. Function must: Return void, and take 1 parameter of type ActorBase*
 *
 * @param func	- A void function that takes only one ActorBase* parameter.
 */
void Gamespace::apply_to_npc(void (Gamespace::*func)(NPC*))
{
	for ( auto& it : _hostile ) (this->*func)(&it); // Apply to enemies
	for ( auto& it : _neutral ) (this->*func)(&it); // Apply to neutrals
}

/**
 * apply_to_npc(bool(Gamespace::*)(NPC*))
 * @brief Applies a Gamespace function to all NPC actors. Function must: Return void, and take 1 parameter of type ActorBase*
 *
 * @param func	- A void function that takes only one ActorBase* parameter.
 */
void Gamespace::apply_to_npc(bool (Gamespace::*func)(NPC*))
{
	for ( auto& it : _hostile ) (this->*func)(&it); // Apply to enemies
	for ( auto& it : _neutral ) (this->*func)(&it); // Apply to neutrals
}
#pragma endregion		GAME_APPLY_TO_TYPE
// Gamespace functions that return private/protected variables.
#pragma region GAME_GETTERS
/**
 * get_all_actors()
 * @brief Returns a vector of pointers containing all actors in the game.
 *
 * @returns vector<ActorBase*>
 */
std::vector<ActorBase*> Gamespace::get_all_actors()
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
std::vector<NPC*> Gamespace::get_all_npc()
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
std::vector<ItemStaticBase*> Gamespace::get_all_static_items()
{
	std::vector<ItemStaticBase*> allItems;
	allItems.reserve(_item_static_health.size() + _item_static_stamina.size());
	for ( auto& it : _item_static_health ) allItems.emplace_back(&it);
	for ( auto& it : _item_static_stamina ) allItems.emplace_back(&it);
	return allItems;
}
/**
 * getClosestActor(Coord&, int)
 * @brief Returns a pointer to the closest actor to a given position.
 *
 * @param pos		- Position ref
 * @param visRange	- Range to check in all directions around the position ref
 * @returns ActorBase*
 */
ActorBase* Gamespace::getClosestActor(const Coord& pos, const int visRange)
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
ActorBase* Gamespace::getActorAt(const Coord& pos)
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
ActorBase* Gamespace::getActorAt(const unsigned int posX, const unsigned int posY)
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
ItemStaticBase* Gamespace::getItemAt(const Coord& pos)
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
ItemStaticBase* Gamespace::getItemAt(const unsigned int posX, const unsigned int posY)
{
	for ( auto* it : get_all_static_items() ) {
		if ( posX == it->pos()._x && posY == it->pos()._y ) return it;
	}
	return nullptr;
}
/**
 * getPlayer()
 * @brief Returns a reference to the player instance.
 * @returns Player&
 */
Player& Gamespace::getPlayer() { return _player; }
/**
 * getTile(Coord&)
 * @brief Returns a reference to the tile at a given position
 *
 * @param pos		- Target coordinate in the tile matrix
 * @returns Tile&	- Reference of tile at pos, or __TILE_ERROR if an invalid coordinate was received.
 */
//Tile* Gamespace::getTile(const Coord& pos) { return _world.get(pos); }
/**
 * getTile(int, int)
 * @brief Returns a reference to the tile at a given position
 *
 * @param x			- Target X-axis coordinate in the tile matrix
 * @param y			- Target Y-axis coordinate in the tile matrix
 * @returns Tile&	- Reference of tile at pos, or __TILE_ERROR if an invalid coordinate was received.
 */
//Tile* Gamespace::getTile(const unsigned int x, const unsigned int y) { return _world.get(x, y); }
/**
 * getCell()
 * @brief Returns a reference to the attached cell
 * @returns Cell&
 */
Cell& Gamespace::getCell() { return { _world }; }
/**
 * getCellSize()
 * @brief Returns a copy of the cell's size
 * @returns Coord
 */
Coord Gamespace::getCellSize() const { return _world._max; }
/**
 * getRuleset()
 * @brief Returns a reference to the attached GameRules instance.
 * @returns GameRules&
 */
GameRules& Gamespace::getRuleset() const { return _ruleset; }
#pragma endregion			GAME_GETTERS
// Gamespace functions that apply passive effects to all actors, such as level-ups & stat regeneration.
#pragma region GAME_PASSIVE_EFFECTS
/**
 * regen(ActorBase*)
 * @brief Increases an actors stats by the relevant value in ruleset.
 *
 * @param actor	- Pointer to an actor
 */
// ReSharper disable once CppMemberFunctionMayBeConst
void Gamespace::regen(ActorBase* actor)
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
void Gamespace::regen(ActorBase* actor, int percent)
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
void Gamespace::level_up(ActorBase* a)
{
	// return if nullptr was passed
	if ( a != nullptr && _ruleset.canLevelUp(a) ) {
		a->addLevel();
		// If actor who leveled up is the player
		if ( a->faction() == FACTION::PLAYER ) {
			regen(&*a, _ruleset._level_up_restore_percent);
			addFlare(_FLARE_DEF_LEVEL);
		}
	}
}

/**
 * apply_level_ups()
 * @brief Applies pending level ups to all actors.
 */
void Gamespace::apply_level_ups() { apply_to_all(&Gamespace::level_up); }

/**
 * apply_passive()
 * @brief Applies passive regen effects to all actors. Amounts are determined by the ruleset.
 */
void Gamespace::apply_passive() { apply_to_all(&Gamespace::regen); }

#pragma endregion	GAME_PASSIVE_EFFECTS
// Gamespace functions that move actors, or are related to moving actors.
#pragma region GAME_MOVE_FUNCTIONS
/**
 * getRandomDir()
 * @brief Returns a random direction char
 * @returns char	- w/a/s/d
 */
char Gamespace::getRandomDir()
{
	return __controlset->intToDir(_rng.get(3, 0));
}

/**
 * canMove(Coord)
 * @brief Returns true if the target position can be moved to, and there is not an actor currently occupying it.
 *
 * @param pos	 - Target position
 * @returns bool - ( false = cannot move ) ( true = can move )
 */
bool Gamespace::canMove(const Coord& pos)
{
	return _world.canMove(pos) && getActorAt(pos) == nullptr ? true : false;
}

/**
 * canMove(int, int)
 * @brief Returns true if the target position can be moved to, and there is not an actor currently occupying it.
 *
 * @param posX	 - Target X position
 * @param posY	 - Target Y position
 * @returns bool - ( false = cannot move ) ( true = can move )
 */
bool Gamespace::canMove(const int posX, const int posY)
{
	return _world.canMove(posX, posY) && getActorAt(posX, posY) == nullptr ? true : false;
}

/**
 * canMove(Coord&, NPC*)
 * @brief Variant of the canMove() function designed for NPCs. Checks for NPCs of the same faction at the target location.
 *
 * @param pos	 - Target position
 * @param myFac	 - The actor who wants to move to target's faction
 * @returns bool - ( false = cannot move ) ( true = can move )
 */
bool Gamespace::checkMove(const Coord& pos, const FACTION myFac)
{
	if ( _world.canMove(pos) ) {
		// check pos for an actor
		auto* target{ getActorAt(pos) };
		// if there is no target, or if there is a target not of my faction
		if ( target == nullptr || target != nullptr && myFac != target->faction() )
			return true; // can move to this tile
	} // else
	return false; // cannot move to this tile
}

/**
 * move(ActorBase*, char)
 * @brief Attempts to move the target actor to an adjacent tile, and processes trap & item logic.
 *
 * @param actor				  - A pointer to the target actor
 * @param dir				  - (w = up / s = down / a = left / d = right) all other characters are ignored.
 * @returns bool - ( true = moved successfully ) ( false = did not move )
 */
bool Gamespace::move(ActorBase* actor, const char dir)
{
	auto did_move{ false };
	if ( actor != nullptr ) {
		auto* target{ getActorAt(actor->getPosDir(dir)) }; // declare a pointer to potential attack target

		// If the actor killed someone with an attack, move them to the target tile.
		if ( target != nullptr && (attack(actor, target) == 1 && canMove(target->pos())) ) {  // NOLINT(bugprone-branch-clone)
			actor->moveDir(dir);
			did_move = true;
		}
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
		if ( _world.isTrap(actor->pos()) ) {
			if ( actor->faction() == FACTION::PLAYER && _ruleset._player_godmode )
				return did_move;
			if ( _ruleset._trap_percentage ) actor->modHealth(-static_cast<int>(static_cast<float>(actor->getMaxHealth()) * (static_cast<float>(_ruleset._trap_dmg) / 100.0f)));
			else actor->modHealth(-_ruleset._trap_dmg);
		}
	}
	return did_move;
}

/**
 * moveNPC(NPC*, bool)
 * @brief Attempt to move an npc with obstacle avoidance towards its current target.
 *
 * @param npc	 - Pointer to an NPC instance
 * @param noFear - NPC will never run away
 */
bool Gamespace::moveNPC(NPC* npc, const bool noFear)
{
	auto dir{ npc->getDirTo(noFear) };
	const auto dirAsInt{ __controlset->dirToInt(dir) };
	// if NPC can move in their chosen direction, return result of move
	if ( checkMove(npc->getPosDir(dir), npc->faction()) )
		return move(&*npc, dir);
	// else find a new direction
	switch ( _rng.get(1, 0) ) { // randomly choose order
	case 0: // check adjacent tiles clockwise
		for ( auto it{ dirAsInt - 1 }; it <= dirAsInt + 1; it += 2 ) {
			// check if iterator is a valid direction int
			if ( it >= 0 && it <= 3 ) {
				dir = __controlset->intToDir(it);
			}
			// check if iterator went below 0, and correct it
			else if ( it == -1 ) {
				dir = __controlset->intToDir(3);
			}
			// check if iterator went above 3, and correct it
			else if ( it == 4 ) {
				dir = __controlset->intToDir(0);
			}
			else continue; // undefined
			if ( checkMove(npc->getPosDir(dir), npc->faction()) )
				return move(&*npc, dir);
		}
		break;
	case 1: // check adjacent tiles counter-clockwise
		for ( auto it{ dirAsInt + 1 }; it >= dirAsInt - 1; it -= 2 ) {
			// check if iterator is a valid direction int
			if ( it >= 0 && it <= 3 ) {
				dir = __controlset->intToDir(it);
			}
			// check if iterator went below 0, and correct it
			else if ( it == -1 ) {
				dir = __controlset->intToDir(3);
			}
			// check if iterator went above 3, and correct it
			else if ( it == 4 ) {
				dir = __controlset->intToDir(0);
			}
			else continue;
			if ( checkMove(npc->getPosDir(dir), npc->faction()) )
				return move(&*npc, dir);
		}
		break;
	default:break;
	}
	// failed, return false
	return false;
}

#pragma endregion		GAME_MOVE_FUNCTIONS
// Gamespace functions that process actor vs. actor combat.
#pragma region GAME_ATTACK
/**
 * attack(ActorBase*, ActorBase*)
 * @brief Allows an actor to attack another actor.
 *
 * @param attacker		- A pointer to the attacking actor
 * @param target		- A pointer to the actor being attacked
 * @returns int		- ( -1 = attack failed ) ( 0 = success, target is still alive ) ( 1 = success, target killed )
 */
int Gamespace::attack(ActorBase* attacker, ActorBase* target)
{
	// return early if target is player and godmode is enabled
	if ( _ruleset._player_godmode && target->faction() == FACTION::PLAYER )	return -1;
	// damage is a random value between the actor's max damage, and (max damage / 6)
	const auto dmg = _rng.get(attacker->getMaxDamage(), attacker->getMaxDamage() / 6);
	// FULL-ATTACK (actor has enough stamina) - Can't parry
	if ( attacker->getStamina() >= _ruleset._attack_cost_stamina ) {
		attacker->modStamina(-_ruleset._attack_cost_stamina);
		// if target has enough stamina, roll for block
		if ( target->getStamina() >= _ruleset._attack_cost_stamina && _rng.get(_ruleset._attack_block_chance, 0.0f) < 1.0f ) {
			target->modHealth(-(dmg / 10));
		}
		else target->modHealth(-dmg);
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
	if ( target->isDead() ) {
		attacker->addKill(target->getLevel() > attacker->getLevel() ? target->getLevel() - attacker->getLevel() : 1);
		if ( target->faction() == FACTION::PLAYER ) {
			_game_state._game_is_over.store(true);
			_game_state._playerDead.store(true);
			_game_state._player_killed_by = attacker->name();
		}
	}
	// if attacker died from parry
	else if ( attacker->isDead() )
		target->addKill(attacker->getLevel() > target->getLevel() ? attacker->getLevel() - target->getLevel() : 1);
	else target->setRelationship(attacker->faction(), true);
	if ( _ruleset._player_godmode && attacker->faction() == FACTION::PLAYER )
		attacker->modStamina(_ruleset._attack_cost_stamina);
	return target->isDead();
}
#pragma endregion				GAME_ATTACK
// Gamespace functions that perform actions for NPCs
#pragma region GAME_ACTION_NPC
/**
 * actionNPC(NPC*)
 * @brief Performs an action for a single NPC
 *
 * @param npc	 - Pointer to an NPC instance
 * @returns bool - ( true = NPC moved ) ( false = NPC did not move )
 */
bool Gamespace::actionNPC(NPC* npc)
{
	auto rc{ false }; // return code
	// Finale Challenge Event - enemies always attack player, neutrals attack player if ruleset allows it
	if ( _game_state._final_challenge.load() && (npc->faction() == FACTION::ENEMY || npc->faction() == FACTION::NEUTRAL && _ruleset._challenge_neutral_is_hostile) ) {
		if ( !npc->isAggro() || npc->getTarget() != &_player ) { // during the final challenge event, force all Enemies to be aggravated against the player.
			npc->maxAggroTarget(&_player);
		}
		rc = moveNPC(&*npc, true);
	}
	else { // Normal turn
		// if the npc is hostile to player, and can see them, switch targets
		if ( npc->canSeeHostile(&_player) ) {
			//npc->setTarget(&_player);
			if ( npc->maxAggroTarget(&_player) )
				rc = moveNPC(&*npc);
		}
		// npc is aggravated
		else if ( npc->isAggro() && _rng.get(_ruleset._npc_move_chance_aggro, 0.0f) >= 1.0f ) {
			// if the NPC has a target, move to it
			if ( npc->hasTarget() )
				rc = moveNPC(&*npc);
			else npc->removeAggro();

			// If the NPC can still see their target, set aggression to max and continue following
			if ( npc->canSeeTarget(_ruleset._npc_vis_mod_aggro) )
				npc->maxAggro();
			npc->decrementAggro();
		}
		// npc is idle, check nearby
		else {
			// get a pointer to the nearest actor
			auto* const nearest{ getClosestActor(npc->pos(), npc->getVis()) };
			if ( nearest != nullptr ) {
				if ( npc->canSeeHostile(&*nearest) ) {
				//	npc->setTarget(&*nearest);
					if ( npc->maxAggroTarget(&*nearest) )
						rc = moveNPC(&*npc);
				}
				else if ( _rng.get(_ruleset._npc_move_chance, 0.0f) < 1.0f )
					rc = move(&*npc, getRandomDir());
			}
			else if ( _rng.get(_ruleset._npc_move_chance, 0.0f) < 1.0f )
				rc = move(&*npc, getRandomDir());
		}
	}
	return rc;
}

/**
 * actionAllNPC()
 * @brief Performs an action for all NPC instances
 */
void Gamespace::actionAllNPC()
{
	// Check if the final challenge should be triggered
	if ( trigger_final_challenge(_hostile.size()) && !_game_state._final_challenge.load() ) {
		_game_state._final_challenge.store(true);
		addFlare(_FLARE_DEF_CHALLENGE);
		if ( !_ruleset._boss_spawns_after_final ) {
			_game_state._boss_challenge.store(true);
			spawn_boss();
		}
	}
	// Perform all NPC actions.
	apply_to_npc(&Gamespace::actionNPC);
}
#pragma endregion			GAME_ACTION_NPC
// Gamespace functions that perform actions for the player.
#pragma region GAME_ACTION_PLAYER
/**
 * actionPlayer(char)
 * @brief Moves the player in a given direction, if possible.
 *
 * @param key	- 'w' for up, 's' for down, 'a' for left, 'd' for right. Anything else is ignored.
 */
void Gamespace::actionPlayer(const char key)
{
	// if not dead and move was successful
	if ( !_player.isDead() && move(&_player, key) ) {
		if ( _ruleset._dark_mode ) _world.modVis(false);
		// player specific post-movement functions
		_world.modVisCircle(true, _player.pos(), _player.getVis() + 2); // allow the player to see the area around them
	}
}
#pragma endregion		GAME_ACTION_PLAYER
// Gamespace functions related to cleaning up expired game elements.
#pragma region GAME_CLEANUP
/**
 * cleanupDead()
 * @brief Cleans up expired game elements. This is called by the frame buffer before every frame.
 */
void Gamespace::cleanupDead()
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
	if ( _player.isDead() ) {
		_game_state._game_is_over.store(true);
		_game_state._playerDead.store(true);
	}
	if ( _hostile.empty() ) {
		if ( !_game_state._boss_challenge ) {
			spawn_boss();
		}
		else {
			_game_state._game_is_over.store(true);
			_game_state._allEnemiesDead.store(true);
		}
	}
}
#pragma endregion			GAME_CLEANUP
// Gamespace functions related to FrameBuffer color flares.
#pragma region GAME_FLARE
/**
 * changeFlare(Flare&)
 * Resets the current flare instance, and applies a new one.
 * This function should be used rather than directly accessing the _flare pointer.
 *
 * @param newFlare	- A new flare instance.
 */
void Gamespace::addFlare(Flare& newFlare)
{
	_FLARE_QUEUE.push_back(&newFlare);
}

/**
 * getFlare()
 * @brief Returns the current flare pointer.
 *
 * @returns Flare*	- Pointer to the currently active flare.
 */
Flare* Gamespace::getFlare() const { return !_FLARE_QUEUE.empty() ? _FLARE_QUEUE.at(0) : nullptr; }

/**
 * resetFlare()
 * @brief Resets & de-initializes the currently set flare instance.
 */
void Gamespace::resetFlare()
{
	if ( !_FLARE_QUEUE.empty() ) {
		_FLARE_QUEUE.at(0)->reset();
		_FLARE_QUEUE.erase(_FLARE_QUEUE.begin());
	} // else do nothing
}
#pragma endregion				GAME_FLARE