#pragma once
#include "../actors/ActorBase.hpp"
#include "matrix.hpp"
#include "../GameConfig.h"

#include <math.hpp>
#include <xRand.hpp>

#include <optional>
#include <map>

struct gamespace {
	rng::Random rng;
	matrix grid;
	Player player;
	std::vector<std::unique_ptr<NPC>> npcs;

	gamespace() : grid{ rng, GameConfig.gridSize, GameConfig.generatorConfig }, player{ [this]() {
		const auto& cont{ getAllValidSpawnTiles<floortile>(false) };
		return cont.at(rng.get(0, cont.size()));
	}(), GameConfig.player_template }
	{
		generate<NPC>(GameConfig.generate_npc_count, GameConfig.npc_templates);
		generate<NPC>(GameConfig.generate_enemy_count, GameConfig.enemy_templates);
	}

	template<std::derived_from<tile>... ValidSpawnTiles>
	std::vector<point> getAllValidSpawnTiles(const bool& checkForActors = true)
	{
		std::vector<point> vec;
		vec.reserve(grid.Size);

		size_t i{ 0ull };
		for (const auto& it : grid) {
			if (auto* tile{ it.get() }; var::variadic_or(typeid(*tile) == typeid(ValidSpawnTiles)...)) {
				point pos{ grid.from1D(i) };
				if (!checkForActors || getActorAt(pos) == nullptr)
					vec.emplace_back(std::move(pos));
			}
			++i;
		}

		vec.shrink_to_fit();
		return vec;
	}
	std::vector<point> getAllValidSpawnTiles(const bool& checkForActors = true)
	{
		return getAllValidSpawnTiles<floortile>(checkForActors);
	}

	/**
	 * @brief			Select a random actor template from a given vector using the random number generator and exponential distribution.
	 * @param templates	A vector of type (ActorTemplate), sorted so that the lowest
	 */
	ActorTemplate getRandomActorTemplate(const std::vector<ActorTemplate>& templates)
	{
		std::exponential_distribution<> dist{ GameConfig.npcDistribRate };
		return templates.at([&dist, &templates, this]() -> size_t {
			const auto& out{ dist(rng.getEngine()) };
			const double& sizef{ static_cast<double>(templates.size() - 1ull) };
			const auto& min{ dist.min() }, max{ 1.0 };
			const auto& normalized{ std::round(math::normalize(std::fmod(out, max), std::make_pair(min, max), std::make_pair(0.0, sizef))) };
			const auto& indexf{ static_cast<float>(normalized) };
			return static_cast<size_t>(indexf);
		}());
	}

	template<std::derived_from<ActorBase> T>
	void generate(const size_t& count, const std::vector<ActorTemplate>& templates)
	{
		auto posCandidates{ getAllValidSpawnTiles() };
		// Gets a random point from the list of valid candidates, and removes the returned point from the list.
		const auto& getRandomPos{ [&posCandidates, this]() -> point {
			const auto& index{ rng.get(0ull, posCandidates.size() - 1ull) };
			const auto& pos{ posCandidates.at(index) };
			const auto& indexIt{ posCandidates.begin() + index };
			posCandidates.erase(indexIt, indexIt);
			return pos;
		} };
		npcs.reserve(npcs.size() + count);

		for (size_t i{ 0ull }; i < count; ++i)
			npcs.emplace_back(std::make_unique<T>(getRandomPos(), getRandomActorTemplate(templates)));

		npcs.shrink_to_fit();
	}

	ActorBase* getActorAt(const point& pos) const
	{
		if (pos == player.pos)
			return (ActorBase*)&player;

		for (const auto& it : npcs)
			if (it.get() != nullptr)
				if (it->pos == pos)
					return (ActorBase*)it.get();

		return nullptr;
	}
	ActorBase* getActorAt(const position& x, const position& y) const
	{
		if (const auto& playerPos{ player.pos }; x == playerPos.x && y == playerPos.y)
			return (ActorBase*)&player;

		for (const auto& it : npcs)
			if (it.get() != nullptr)
				if (const auto& pos{ it->pos }; pos.x == x && pos.y == y)
					return (ActorBase*)it.get();

		return nullptr;
	}

	tile* getTileAt(const point& pos) const
	{
		return grid.get(pos);
	}
	tile* getTileAt(const position& x, const position& y) const
	{
		return grid.get(x, y);
	}

	std::tuple<tile*, ActorBase*> getAt(const point& pos) const
	{
		return{ getTileAt(pos), getActorAt(pos) };
	}
	std::tuple<tile*, ActorBase*> getAt(const position& x, const position& y) const
	{
		return{ getTileAt(x, y), getActorAt(x, y) };
	}

	template<std::derived_from<tile>... Ts>
	bool checkTileType(tile* t)
	{
		const auto& tType{ typeid(*t) };
		return var::variadic_or(tType == typeid(Ts)...);
	}

	bool canMove(ActorBase* actor, const point& posDiff) noexcept(false)
	{
		if (actor == nullptr)
			throw make_exception("gamespace::canMove() failed:  Received nullptr instead of ActorBase*!");
		const auto& newPos{ actor->pos + posDiff };
		if (!newPos.within(getPlayableBounds())) // if the pos is out of bounds, return false early
			return false;
		if (auto* tile{ getTileAt(newPos) }; tile != nullptr && typeid(*tile) != typeid(walltile)) {
			// check for actors
			if (const auto& other{ getActorAt(newPos) }; other != nullptr) {
				// check if the actor is hostile towards the target
				if (GameConfig.getFactionFromID(actor->factionID).isHostileTo(other->factionID)) {
					return other->applyDamageFrom(actor); // return true if other died
				}
				else return false;
			}
			return true;
		}
		return false;
	}

#	pragma region Movement
	/**
	 * @brief			Move an actor by the specified number of positions.
	 * @param actor		The actor to move.
	 * @param posDiff	A point to add to the actors current position.
	 * @returns			bool
	 *					true:	Successfully moved actor by the specified distance.
	 *					false:	Failed to move the actor.
	 */
	bool moveActor(ActorBase* actor, const point& posDiff)
	{
		if (canMove(actor, posDiff)) {
			actor->movePosBy(posDiff);
			if (auto* tile{ getTileAt(actor->pos) }; tile != nullptr)
				tile->effect(actor);
			return true;
		}
		return false;
	}
	/**
	 * @brief			Move an actor by the specified number of positions.
	 * @param actor		The actor to move.
	 * @param posDiff	A point to add to the actors current position.
	 * @returns			bool
	 *					true:	Successfully moved actor by the specified distance.
	 *					false:	Failed to move the actor.
	 */
	bool moveActor(ActorBase* actor, point&& posDiff)
	{
		if (canMove(actor, std::forward<point>(posDiff))) {
			actor->movePosBy(std::forward<point>(posDiff));
			if (auto* tile{ getTileAt(actor->pos) }; tile != nullptr)
				tile->effect(actor);
			return true;
		}
		return false;
	}
	/**
	 * @brief			Move an actor by the specified number of positions.
	 * @param actor		The actor to move.
	 * @param x			X-Axis Position Modifier
	 * @param y			Y-Axis Position Modifier
	 * @returns			bool
	 *					true:	Successfully moved actor by the specified distance.
	 *					false:	Failed to move the actor.
	 */
	bool moveActor(ActorBase* actor, const position& x, const position& y)
	{
		return moveActor(actor, std::move(point{ x, y }));
	}

	bool movePlayer(const point& posDiff) { return moveActor(&player, posDiff); }
	bool movePlayer(const position& x, const position& y) { return moveActor(&player, x, y); }
#	pragma endregion Movement

	Faction& getFaction(const int& factionID)
	{
		return GameConfig.getFactionFromID(factionID);
	}

	/**
	 * @brief		Remove an NPC from the list using a given iterator.
	 *\n			If you're calling this from a for loop, make sure to set the given
	 *\n			iterator to the return value of this function to prevent a hanging pointer.
	 * @param it	An iterator pointing to the target npc.
	 * @returns		std::vector<std::unique_ptr<NPC>>::iterator
	 */
	std::vector<std::unique_ptr<NPC>>::iterator removeNPC(const std::vector<std::unique_ptr<NPC>>::const_iterator& it)
	{
		return npcs.erase(it, it + 1);
	}

	point pathFind(const point& start, const point& target)
	{
		const auto& bounds{ getPlayableBounds() };
		const auto& movable{ [&bounds, this](const point& p) {
			if (p.within(bounds))
				return tileAllowsMovement(getTileAt(p));
			return false;
		} };

		const point& diff{ start.distanceTo(target) }, diffNormal{ diff.clamp() };

		if (const auto& sml{ diff.getSmallestAxis() }; sml == 0 || rng.get(0, 2) == 0) { // 33% chance of using small axis
			const auto& lrg{ diff.getLargestAxis() };
			if (lrg == diff.x)
				return point{ lrg, 0 }.clamp();
			else
				return point{ 0, lrg }.clamp();
		}
		else if (sml == diff.x)
			return point{ sml, 0 }.clamp();
		else
			return point{ 0, sml }.clamp();

		return { 0, 0 };
	}
	point pathFind(ActorBase* actor, const point& target)
	{
		return pathFind(actor->pos, target);
	}

	bool PerformActionNPC(NPC* npc)
	{
		if (npc == nullptr)
			throw make_exception("gamespace::PerformActionNPC() failed:  Received nullptr!");

		//if (npc->hasTarget()) {
		//	if (auto* target{ npc->getTarget() }; target != nullptr) {
		//		if (target->isDead()) // unset dead target
		//			npc->setTarget(nullptr);
		//		// else target is alive
		//		else {
		//			moveActor(npc, pathFind(npc, target->pos));
		//			return npc->isDead();
		//		}
		//	}
		//}
		// check nearby positions for enemies
		const auto& myFaction{ getFaction(npc->factionID) };
		const auto& nearby{ npc->pos.getAllPointsWithinCircle(npc->aggressionRange, getPlayableBounds()) };
		for (const auto& npos : nearby) {
			if (auto* actor{ getActorAt(npos) }; actor != nullptr && myFaction.isHostileTo(actor->factionID)) {
				//npc->setTarget(actor);
				moveActor(npc, pathFind(npc, actor->pos));
				return npc->isDead();
			}
		}
		if (rng.get(0.0f, 100.0f) <= GameConfig.npcIdleMoveChance) {
			point dir{ 0, 0 };
			if (rng.get(0, 1) == 1)
				dir.x += rng.get(-1, 1);
			else
				dir.y += rng.get(-1, 1);
			moveActor(npc, dir);
		}
		return npc->isDead();
	}

	/**
	 * @brief	Performs an action for all npcs in the list.
	 *\n		This function should be called by the game loop.
	 */
	void PerformActionAllNPCs()
	{
		for (auto it{ npcs.begin() }; it != npcs.end(); ) {
			if (auto* npc{ it->get() }; npc != nullptr) {
				if (npc->isDead() || PerformActionNPC(npc)) {
					it->reset();
					it = removeNPC(it);
					continue; ///< required!
				}
			}
			else it = removeNPC(it);
			if (it != npcs.end())
				++it;
		}
	}
};
