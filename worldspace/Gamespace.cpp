#include "Gamespace.h"
// Gamespace Constructor
#pragma region GAME_CONSTRUCTOR
/** CONSTRUCTOR **
 * Gamespace(GLOBAL&, GameRules&)
 * @brief Creates a new gamespace with the given settings.
 *
 * @param ruleset	 - A ref to the ruleset structure
 */
Gamespace::Gamespace(GameRules& ruleset) : _world(!ruleset._world_import_file.empty() ? Cell{ ruleset._world_import_file, ruleset._walls_visible, ruleset._all_tiles_visible } : Cell{ ruleset._cellSize, ruleset._walls_visible, ruleset._all_tiles_visible }), _ruleset(ruleset), _flare(nullptr)
{
	_actors.emplace_back(new Player(findValidSpawn(true), _ruleset._player_template));
	_world.modVisCircle(true, _actors.front()->pos(), _actors.front()->getVis() + 2); // allow the player to see the area around them
	populate_actor_vec<Enemy>(_ruleset._enemy_count, _ruleset._enemy_template);
	populate_actor_vec<Neutral>(_ruleset._neutral_count, _ruleset._neutral_template);
	populate_item_vec<ItemStaticHealth>(_ruleset._item_health_count, _ruleset._item_health_restore, _ruleset._item_health_faction_lock);
	populate_item_vec<ItemStaticStamina>(_ruleset._item_stamina_count, _ruleset._item_stamina_restore, _ruleset._item_stamina_faction_lock);
//	_hostile = generate_NPCs<Enemy>(ruleset._enemy_count, ruleset._enemy_template);
//	_neutral = generate_NPCs<Neutral>(ruleset._neutral_count, ruleset._neutral_template);
//	_item_static_health = generate_items<ItemStaticHealth>(10);
//	_item_static_stamina = generate_items<ItemStaticStamina>(10);

#ifdef _DEBUG
	_game_state._final_challenge.store(true);
	_flare = std::make_unique<Flare>(FlareChallenge(_world._max));
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
		for ( auto findPos{ pos }; !_world.get(findPos)._canSpawn; pos = findPos )
			findPos = { _rng.get(_world._max._x - 2, 1), _rng.get(_world._max._y - 2, 1) };
		// Check the distance from player
		if ( !isPlayer ) {
			const auto player{ getPlayer() };
			if ( (getActorAt(pos) == nullptr ? true : false) && (!checkForItems ? true : getItemAt(pos) == nullptr ? true : false) && player->getDist(pos) <= _ruleset._enemy_aggro_distance + player->getVis() )
				return pos;
		}
		// Don't check the distance from player
		else if ( (getActorAt(pos) == nullptr ? true : false) && (!checkForItems ? true : getItemAt(pos) == nullptr ? true : false) )
			return pos;
	}
	// Else return invalid coord
	return { -1, -1 };
}
template<typename NPC_Type>
void Gamespace::populate_actor_vec(const unsigned count, std::vector<ActorTemplate>& templates)
{
	// reserve more space
	_actors.reserve(_actors.size() + count);
	
	for ( auto i{ 0 }; i < count; ++i ) {
		for ( auto it{ static_cast<signed>(templates.size() - 1) }; it >= 0; --it ) {
			// if random number between 0.0 and 100.0 is less than the template's chance
			if ( _rng.get(100.0f, 0.0f) < templates.at(it)._chance ) {
				_actors.emplace_back(new NPC_Type(findValidSpawn(), templates.at(it)));
				break; // stop iterating through templates & continue
			}
		}
	}
}
template <typename ItemType>
void Gamespace::populate_item_vec(const unsigned count, const int amount, std::vector<FACTION> factionLock)
{
	_items.reserve(_items.size() + count);

	for ( auto i{ 0 }; i < count; ++i ) {
		_items.emplace_back(new ItemType(findValidSpawn(), amount, factionLock));
	}
}

#pragma endregion			GAME_SPAWNING
// Gamespace functions that apply other functions to multiple types of objects.
#pragma region GAME_APPLY_TO_TYPE
/**
 * apply_to_all(void(Gamespace::*)(std::shared_ptr<ActorBase>))
 * @brief Applies a Gamespace function to all actors. Function must: Return void, and take 1 parameter of type std::shared_ptr<ActorBase>
 *
 * @param func	- A void function that takes only one std::shared_ptr<ActorBase> parameter.
 */
void Gamespace::apply_to_all(void (Gamespace::*func)(const std::shared_ptr<ActorBase>&))
{
	for ( auto& it : _actors ) (this->*func)(it);
}

/**
 * apply_to_npc(void(Gamespace::*)(std::shared_ptr<NPC>))
 * @brief Applies a Gamespace function to all NPC actors. Function must: Return void, and take 1 parameter of type std::shared_ptr<ActorBase>
 *
 * @param func	- A void function that takes only one std::shared_ptr<ActorBase> parameter.
 */
void Gamespace::apply_to_npc(void (Gamespace::*func)(const std::shared_ptr<NPC>&))
{
	for ( auto& it : _actors )
		if ( it->faction() != FACTION::PLAYER )
			(this->*func)(std::static_pointer_cast<NPC>(it));
}

/**
 * apply_to_npc(bool(Gamespace::*)(std::shared_ptr<NPC>))
 * @brief Applies a Gamespace function to all NPC actors. Function must: Return void, and take 1 parameter of type std::shared_ptr<ActorBase>
 *
 * @param func	- A void function that takes only one std::shared_ptr<ActorBase> parameter.
 */
void Gamespace::apply_to_npc(bool (Gamespace::*func)(const std::shared_ptr<NPC>&))
{
	for ( auto& it : _actors )
		if ( it->faction() != FACTION::PLAYER )
			(this->*func)(std::static_pointer_cast<NPC>(it));
}
#pragma endregion		GAME_APPLY_TO_TYPE
// Gamespace functions that return private/protected variables.
#pragma region GAME_GETTERS
/**
 * get_all_npc()
 * @brief Returns a vector of pointers containing all NPC actors in the game.
 *
 * @returns vector<std::shared_ptr<NPC>>
 */
std::vector<std::shared_ptr<NPC>> Gamespace::get_all_npc()
{
	std::vector<std::shared_ptr<NPC>> vec;
	vec.reserve(_enemy_count + _neutral_count);
	for ( auto& it : _actors )
		if ( it->faction() != FACTION::PLAYER )
			vec.emplace_back(it);
	return vec;
}

/**
 * get_all_static_items()
 * @brief Returns a vector of pointers containing all actors in the game.
 *
 * @returns vector<ItemStaticBase*>
 */
std::vector<std::shared_ptr<ItemStaticBase>> Gamespace::get_all_static_items()
{
	std::vector<std::shared_ptr<ItemStaticBase>> vec;
	for ( auto& it : _items ) {
		vec.emplace_back(it);
	}
	return vec;
}
/**
 * getNearbyActor(Coord&, int)
 * @brief Returns a pointer to the closest actor to a given position.
 *
 * @param pos		- Position ref
 * @param visRange	- Range to check in all directions around the position ref
 * @returns std::shared_ptr<ActorBase>
 */
std::shared_ptr<ActorBase> Gamespace::getNearbyActor(const Coord& pos, const int visRange)
{
	std::shared_ptr<ActorBase> nearest{ nullptr };
	auto dist{ 999 };
	for ( auto range{1}; range < visRange && dist > 1; ++range ) {
		for ( auto y{ pos._y - range }; y < pos._y + range; ++y ) {
			for ( auto x{ pos._x - range }; x < pos._x + range; ++x ) {
				if ( x != pos._x && y != pos._y ) {
					const auto actor_here{ getActorAt(x, y) };
					if ( actor_here != nullptr ) {
						const auto actor_dist{ getDist(x, y, actor_here->pos()._x, actor_here->pos()._y) };
						if ( actor_dist < dist ) {
							nearest = actor_here;
							dist = actor_dist;
							if ( dist == 1 ) 
								return nearest;
						}
					}
				}
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
 * @returns std::shared_ptr<ActorBase>	- nullptr if not found
 */
std::shared_ptr<ActorBase> Gamespace::getActorAt(const Coord& pos)
{
	for ( auto& it : _actors ) {
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
 * @returns std::shared_ptr<ActorBase>	- nullptr if not found
 */
std::shared_ptr<ActorBase> Gamespace::getActorAt(const int posX, const int posY)
{
	for ( auto& it : _actors ) {
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
std::shared_ptr<ItemStaticBase> Gamespace::getItemAt(const Coord& pos)
{
	for ( auto& it : _items) {
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
std::shared_ptr<ItemStaticBase> Gamespace::getItemAt(const int posX, const int posY)
{
	for ( auto& it : _items ) {
		if ( posX == it->pos()._x && posY == it->pos()._y ) return it;
	}
	return nullptr;
}
/**
 * getPlayer()
 * @brief Returns a reference to the player instance.
 * @returns Player&
 */
 std::shared_ptr<Player> Gamespace::getPlayer()
{
	 for ( auto& it : _actors )
		 if ( it->faction() == FACTION::PLAYER )
			 return std::static_pointer_cast<Player>(it);
	 return nullptr;
}
/**
 * getTile(Coord&)
 * @brief Returns a reference to the tile at a given position
 *
 * @param pos		- Target coordinate in the tile matrix
 * @returns Tile&	- Reference of tile at pos, or __TILE_ERROR if an invalid coordinate was received.
 */
 Tile& Gamespace::getTile(const Coord& pos) { return _world.get(pos); }
/**
 * getTile(int, int)
 * @brief Returns a reference to the tile at a given position
 *
 * @param x			- Target X-axis coordinate in the tile matrix
 * @param y			- Target Y-axis coordinate in the tile matrix
 * @returns Tile&	- Reference of tile at pos, or __TILE_ERROR if an invalid coordinate was received.
 */
 Tile& Gamespace::getTile(const int x, const int y) { return _world.get(x, y); }
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
 * regen(std::shared_ptr<ActorBase>)
 * @brief Increases an actors stats by the relevant value in ruleset.
 *
 * @param actor	- Pointer to an actor
 */
// ReSharper disable once CppMemberFunctionMayBeConst
void Gamespace::regen(const std::shared_ptr<ActorBase>& actor)
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
 * regen(std::shared_ptr<ActorBase>, int)
 * @brief Increases an actors stats by a percentage.
 *
 * @param actor   - Pointer to an actor
 * @param percent - Percentage to restore ( 0 - 100 )
 */
void Gamespace::regen(const std::shared_ptr<ActorBase>& actor, int percent)
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
 * level_up(std::shared_ptr<ActorBase>)
 * @brief Checks if the given actor has enough kills to level up, then increments their level if they do.
 *
 * @param a	- Pointer to a target actor
 */
void Gamespace::level_up(const std::shared_ptr<ActorBase>& a)
{
	// return if nullptr was passed
	if ( a != nullptr && _ruleset.canLevelUp(a) ) {
		a->addLevel();
		// If actor who leveled up is the player
		if ( a->faction() == FACTION::PLAYER ) {
			regen(a, _ruleset._level_up_restore_percent);
			if ( _flare == nullptr )
				changeFlare(FlareLevel());
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
bool Gamespace::canMove(const int posX, const int posY)
{
	return _world.get(posX, posY)._canMove && getActorAt(posX, posY) == nullptr ? true : false;
}

/**
 * canMove(Coord&, std::shared_ptr<NPC>)
 * @brief Variant of the canMove() function designed for NPCs. Checks for NPCs of the same faction at the target location.
 *
 * @param pos	 - Target position
 * @param myFac	 - The actor who wants to move to target's faction
 * @returns bool - ( false = cannot move ) ( true = can move )
 */
bool Gamespace::checkMove(const Coord& pos, const FACTION myFac)
{
	if ( _world.get(pos)._canMove ) {
		// check pos for an actor
		const auto target{ getActorAt(pos) };
		// if there is no target, or if there is a target not of my faction
		if ( target == nullptr || target != nullptr && myFac != target->faction() )
			return true; // can move to this tile
	} // else
	return false; // cannot move to this tile
}

/**
 * move(std::shared_ptr<ActorBase>, char)
 * @brief Attempts to move the target actor to an adjacent tile, and processes trap & item logic.
 *
 * @param actor				  - A pointer to the target actor
 * @param dir				  - (w = up / s = down / a = left / d = right) all other characters are ignored.
 * @returns bool - ( true = moved successfully ) ( false = did not move )
 */
bool Gamespace::move(const std::shared_ptr<ActorBase>& actor, const char dir)
{
	auto did_move{ false };
	const auto target{ getActorAt(actor->getPosDir(dir)) }; // declare a pointer to potential attack target
	
	// if there is an actor at the target pos, attack them
	if ( target != nullptr && (attack(actor, target) == 1 && canMove(target->pos())) ) {  // NOLINT(bugprone-branch-clone)
		actor->moveDir(dir);
		did_move = true;
	}
	// if target tile can be moved to
	else if ( canMove(actor->getPosDir(dir)) ) {
		actor->moveDir(dir);
		did_move = true;
	}
	// if move failed
	else return did_move;
	// if move succeeded, check for items & traps
	auto item{ getItemAt(actor->pos()) };
	if ( item != nullptr ) {
		item->attempt_use(actor);
	}
	// Calculate trap damage if applicable
	if ( _world.get(actor->pos())._isTrap ) {
		if ( _ruleset._trap_percentage ) actor->modHealth(-static_cast<int>(static_cast<float>(actor->getMaxHealth()) * (static_cast<float>(_ruleset._trap_dmg) / 100.0f)));
		else actor->modHealth(-_ruleset._trap_dmg);
	}
	return did_move;
}

/**
 * moveNPC(std::shared_ptr<NPC>, bool)
 * @brief Attempt to move an npc with obstacle avoidance towards its current target.
 *
 * @param npc	 - Pointer to an NPC instance
 * @param noFear - NPC will never run away
 */
bool Gamespace::moveNPC(const std::shared_ptr<NPC>& npc, const bool noFear)
{
	auto dir{ npc->getDirTo(noFear) };
	const auto dirAsInt{ __controlset->dirToInt(dir) };
	// if NPC can move in their chosen direction, return result of move
	if ( checkMove(npc->getPosDir(dir), npc->faction()) )
		return move(npc, dir);
	// else find a new direction
	switch ( _rng.get(1, 0) ) { // randomly choose order
	case 0: // check adjacent tiles clockwise
		for ( auto it{ dirAsInt - 1 }; it <= dirAsInt + 1; it += 2 ) {
			if ( it >= 0 && it <= 3 ) // check if iterator is a valid direction int
				dir = __controlset->intToDir(it);
			else if ( it == -1 ) // check if iterator went below 0, and correct it
				dir = __controlset->intToDir(3);
			else if ( it == 4 ) // check if iterator went above 3, and correct it
				dir = __controlset->intToDir(0);
			else continue; // undefined
			if ( checkMove(npc->getPosDir(dir), npc->faction()) )
				return move(npc, dir);
		}
		break;
	case 1: // check adjacent tiles counter-clockwise
		for ( auto it{ dirAsInt + 1 }; it >= dirAsInt - 1; it -= 2 ) {
			if ( it >= 0 && it <= 3 ) // check if iterator is a valid direction int
				dir = __controlset->intToDir(it);
			else if ( it == -1 ) // check if iterator went below 0, and correct it ('w' -> 'a')
				dir = __controlset->intToDir(3);
			else if ( it == 4 )  // check if iterator went above 3, and correct it ('a' -> 'w')
				dir = __controlset->intToDir(0);
			else continue;
			if ( checkMove(npc->getPosDir(dir), npc->faction()) )
				return move(npc, dir);
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
 * attack(std::shared_ptr<ActorBase>, std::shared_ptr<ActorBase>)
 * @brief Allows an actor to attack another actor.
 *
 * @param attacker		- A pointer to the attacking actor
 * @param target		- A pointer to the actor being attacked
 * @returns int		- ( -1 = attack failed ) ( 0 = success, target is still alive ) ( 1 = success, target killed )
 */
int Gamespace::attack(const std::shared_ptr<ActorBase>& attacker, const std::shared_ptr<ActorBase>& target)
{
	// return early if target is player and godmode is enabled
	if ( _ruleset._player_godmode && target->faction() == FACTION::PLAYER )	return -1;
	// damage is a random value between the actor's max damage, and (max damage / 6)
	const auto dmg{ _rng.get(attacker->getMaxDamage(), attacker->getMaxDamage() / 6) };
	const auto rng{ _rng.get(100.0f, 0.0f) };
	// FULL-ATTACK (actor has enough stamina) - Can't parry
	if ( attacker->getStamina() >= _ruleset._attack_cost_stamina ) {
		attacker->modStamina(-_ruleset._attack_cost_stamina);
		target->modHealth(-dmg);
	}
	// HALF-ATTACK (actor has stamina but not enough for a full attack) - Block
	else if ( attacker->getStamina() >= _ruleset._attack_cost_stamina / 3 ) {
		// remove stamina from attacker
		attacker->modStamina(-_ruleset._attack_cost_stamina);
		// check if the target has enough stamina to block
		if ( target->getStamina() >= _ruleset._attack_cost_stamina / 2 && rng <= _ruleset._attack_block_chance ) {
			target->modStamina(-_ruleset._attack_cost_stamina / 2);
			target->modHealth(-(dmg / 20));
		}
		else target->modHealth(-(dmg / 2));
	}
	// MINIMAL-ATTACK (actor does not have any stamina) - Parry
	else { // remove stamina from attacker
		attacker->modStamina(-_ruleset._attack_cost_stamina);
		// check if the target has enough stamina to parry
		if ( target->getStamina() >= _ruleset._attack_cost_stamina / 2 && rng <= _ruleset._attack_block_chance * 2 ) {
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
#pragma region			GAME_ACTION_NPC
/**
 * actionNPC(std::shared_ptr<NPC>)
 * @brief Performs an action for a single NPC
 *
 * @param npc	 - Pointer to an NPC instance
 * @returns bool - ( true = NPC moved ) ( false = NPC did not move )
 */
bool Gamespace::actionNPC(const std::shared_ptr<NPC>& npc)
{
	auto rc{ false }; // return code
	const auto rng{ _rng.get(100.0f, 0.0f) };
	// normal action, final challenge has not been triggered.
	if ( !_game_state._final_challenge.load() ) {
		// if npc has & can see their current target
		if ( npc->canSeeTarget() ) {
			npc->maxAggro();
			rc = moveNPC(npc);
		}
		// if npc is currently aggravated
		else if ( npc->isAggro() ) {
			if ( rng <= _ruleset._npc_move_chance_aggro ) {
				rc = moveNPC(npc);
				if ( npc->canSeeTarget() )
					npc->maxAggro();
			} // else do nothing
		}
		// npc is not aggravated, but can see the player and is hostile
		else if ( npc->canSee(getPlayer()) && npc->isHostileTo(FACTION::PLAYER) ) {
			npc->maxAggro(getPlayer());
			rc = moveNPC(npc);
		}
		else {
			const auto nearest{ getNearbyActor(npc->pos(), npc->getVis()) };
			if ( nearest != nullptr ) {
				if ( npc->canSee(nearest) ) {
					npc->maxAggro(nearest);
					rc = moveNPC(npc);
				}
				else if ( rng <= _ruleset._npc_move_chance )
					rc = move(npc, getRandomDir());
			}
			else if ( rng <= _ruleset._npc_move_chance )
				rc = move(npc, getRandomDir());
		}
	}
	// final challenge is active, and NPC is hostile to the player
	else if ( npc->isHostileTo(FACTION::PLAYER) ) {
		if ( rng <= _ruleset._npc_move_chance_aggro ) {// If an NPC is not aggravated, or not targeting the player, change target to player
			if ( !npc->isAggro() || npc->getTarget() != getPlayer() )
				npc->maxAggro(getPlayer());
			rc = moveNPC(npc, true);
		}
	}
	// final challenge is active, and NPC is NOT hostile to the player.
	else if ( rng <= _ruleset._npc_move_chance )
		rc = move(npc, getRandomDir());
	npc->decrementAggro();
	return rc;
}
/**
 * actionAllNPC()
 * @brief Performs an action for all NPC instances
 */
void Gamespace::actionAllNPC()
{
	// Check if the final challenge should be triggered
	if ( trigger_final_challenge(_enemy_count) && (!_game_state._final_challenge.load() || !_game_state._final_challenge_flare_complete.load()) ) {
		_game_state._final_challenge.store(true);
		// attempt to set flare
		if ( changeFlare(FlareChallenge(_world._max)) )
			_game_state._final_challenge_flare_complete.store(true);
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
	const auto player{ getPlayer() };
	// if not dead and move was successful
	if ( !player->isDead() && move(getPlayer(), key) ) {
		// player specific post-movement functions
		_world.modVisCircle(true, player->pos(), player->getVis() + 2); // allow the player to see the area around them
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
	// check items
	for ( auto it{ static_cast<signed>(_items.size() - 1) }; it >= 0; --it )
		if ( _items.at(it)->getUses() <= 0 )
			_items.erase(_items.begin() + it);
	for ( auto it{ static_cast<signed>(_actors.size() - 1) }; it >= 0; --it )
		if ( _actors.at(it)->isDead() )
			_actors.erase(_actors.begin() + it);
	// check win condition
	if ( getPlayer()->isDead() ) {
		_game_state._game_is_over.store(true);
		_game_state._playerDead.store(true);
	}
	if ( _enemy_count == 0 ) {
		_game_state._game_is_over.store(true);
		_game_state._allEnemiesDead.store(true);
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
 * @param flare	- A new flare instance.
 */
template<typename FlareType>
bool Gamespace::changeFlare(FlareType flare)
{
	if ( _flare != nullptr ) {
		if ( _flare->time() > 0 )
			return false;
		_flare->reset();
	}
	_flare = std::make_unique<Flare>(flare);
	return true;
}

/**
 * getFlare()
 * @brief Returns the current flare pointer.
 *
 * @returns Flare*	- Pointer to the currently active flare.
 */
 std::unique_ptr<Flare>& Gamespace::getFlare() { return _flare; }

/**
 * resetFlare()
 * @brief Resets & de-initializes the currently set flare instance.
 */
void Gamespace::resetFlare()
{
	// if flare is already reset
	if ( _flare != nullptr ) {
		if ( _flare->time() == 0 )
			_flare = nullptr;// set the flare pointer to nullptr
		else
			_flare = std::make_unique<FlareClear>(FlareClear());
	}
}
#pragma endregion				GAME_FLARE