#pragma once
#include "../actors/ActorBase.hpp"
#include "../GameConfig.h"
#include "matrix.hpp"

#include <math.hpp>
#include <xRand.hpp>

#include <optional>
#include <map>

inline static void setGridSize(const size& newGridSize)
{
	GameConfig.gridSize = newGridSize;
	GameConfig.minPos = point{ 0, 0 } + static_cast<int>(GameConfig.generatorConfig.wall_always_on_edge);
	GameConfig.maxPos = GameConfig.gridSize - GameConfig.minPos;
}

inline static std::pair<point, point> getPlayableBounds()
{
	return std::make_pair(GameConfig.minPos + !!GameConfig.generatorConfig.wall_always_on_edge, GameConfig.maxPos - +!!GameConfig.generatorConfig.wall_always_on_edge);
}

inline static bool isWithinBounds(const point& pos)
{
	return pos.within(getPlayableBounds());
}

enum class CanMoveResult : unsigned char {
	BLOCKED = 0,
	BLOCKED_FRIEND = 1,
	BLOCKED_ENEMY = 2,
	OPEN = 3,
};

/**
 * @struct	gamespace
 * @brief	Game manager object containing all of the functions used at runtime to operate the game.
 *\n		This object maintains ownership over all game assets, and cleans them up at destruction time.
 */
struct gamespace {
	rng::Random rng;
	matrix grid;
	Player player;
	std::vector<std::unique_ptr<NPC>> npcs;

	/**
	 * @brief	Default Constructor.
	 *\n		This initializes the grid to the size specified by (GameConfig.gridSize).
	 */
	gamespace() : grid{ rng, GameConfig.gridSize, GameConfig.generatorConfig }, player{ [this]() {
		const auto& cont{ getAllValidSpawnTiles<floortile>(true) };
		return cont.at(rng.get(0, cont.size()));
	}(), GameConfig.player_template }
	{
		generate_npcs<NPC>(GameConfig.generate_npc_count, GameConfig.npc_templates);
		generate_npcs<Enemy>(GameConfig.generate_enemy_count, GameConfig.enemy_templates);
	}

	/**
	 * @brief						Retrieve a vector of every point in the grid that is considered empty, and is a valid spawn location for actors.
	 * @tparam ValidSpawnTiles...	List of tile types that should be considered "valid". Only tiles whose type is included here will have their positions included in the list.
	 * @returns						std::vector<point>
	 */
	template<std::derived_from<tile>... ValidSpawnTiles>
	std::vector<point> getAllValidSpawnTiles(const bool& skipActorChecks = false)
	{
		std::vector<point> vec;
		vec.reserve(grid.Size);
		size_t i{ 0ull };
		for (const auto& it : grid) {
			if (auto* tile{ it.get() }; var::variadic_or(typeid(*tile) == typeid(ValidSpawnTiles)...)) {
				if (point pos{ grid.from1D(i) }; skipActorChecks || getActorAt(pos) == nullptr)
					vec.emplace_back(std::move(pos));
			}
			++i;
		}
		vec.shrink_to_fit();
		return vec;
	}
	/**
	 * @brief	Retrieve a vector of every point in the grid that is an empty floortile type.
	 * @returns	std::vector<point>
	 */
	std::vector<point> getAllValidSpawnTiles(const bool& skipActorChecks = false)
	{
		return getAllValidSpawnTiles<floortile>(skipActorChecks);
	}

	/**
	 * @brief			Select a random actor template from a given vector using the random number generator and exponential distribution.
	 * @param templates	A vector of type (ActorTemplate), sorted so that the lowest
	 */
	ActorTemplate getRandomActorTemplate(const std::vector<ActorTemplate>& templates, std::exponential_distribution<double>& distribution)
	{
		return templates.at([&distribution, &templates, this]() -> size_t {
			const auto& out{ distribution(rng.getEngine()) };
			const double& sizef{ static_cast<double>(templates.size() - 1ull) };
			const auto& min{ distribution.min() }, max{ 1.0 };
			const auto& normalized{ std::round(math::normalize(std::fmod(out, max), std::make_pair(min, max), std::make_pair(0.0, sizef))) };
			const auto& indexf{ static_cast<float>(normalized) };
			return static_cast<size_t>(indexf);
		}());
	}

	/**
	 * @brief				Generate the specified number of NPC actors, by inserting them into the (npcs) vector.
	 *\n					Uses exponential distribution to select actor templates.
	 * @tparam ActorType	Any type derived from (NPC), this is the type that should be generated.
	 * @param count			The total number of NPCs to generate.
	 * @param templates		A vector of templates to serve as potential candidates.
	 *\n					This should be sorted so that the most common types are located at low indexes.
	 * @returns				size_t
	 */
	template<std::derived_from<NPC> ActorType>
	size_t generate_npcs(const size_t& count, const std::vector<ActorTemplate>& templates)
	{
		if (count == 0ull || templates.empty())
			return 0ull;

		auto posCandidates{ getAllValidSpawnTiles() };

		if (posCandidates.empty())
			return 0ull;

		// Gets a random point from the list of valid candidates, and removes the returned point from the list.
		const auto& getRandomPos{ [&posCandidates, this]() -> point {
			const auto& index{ rng.get(0ull, posCandidates.size() - 1ull) };
			const auto& pos{ posCandidates.at(index) };
			const auto& indexIt{ posCandidates.begin() + index };
			posCandidates.erase(indexIt, indexIt);
			return pos;
		} };

		auto dist{ rng.exponential_distribution<double>(GameConfig.npcDistribRate) };
		// reserve enough space
		npcs.reserve(npcs.size() + count);
		size_t i{ 0ull };
		for (; i < count && !posCandidates.empty(); ++i)
			npcs.emplace_back(std::make_unique<ActorType>(getRandomPos(), getRandomActorTemplate(templates, dist)));
		npcs.shrink_to_fit();
		return i;
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
		if (auto& playerPos{ player.pos }; x == playerPos.x && y == playerPos.y)
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

	template<typename... Ts>
	bool validActorMoveTile(Ts&&... tiles) const
	{
		return var::variadic_or(typeid(tiles) != typeid(walltile)...);
	}

	CanMoveResult canMoveTo(ActorBase* actor, const point& newPos) const noexcept(false)
	{
		if (actor == nullptr)
			throw make_exception("gamespace::canMove() failed:  Received nullptr instead of ActorBase*!");
		const auto& newPos{ actor->pos + posDiff };
		if (!isWithinBounds(newPos)) // if the pos is out of bounds, return false early
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
	void moveActor(ActorBase* actor, const point& posDiff)
	{
		if (canMove(actor, posDiff)) {
			actor->movePosBy(posDiff);
			if (auto* tile{ getTileAt(actor->pos) }; tile != nullptr)
				tile->effect(actor);
		}
	}
	void moveActor(ActorBase* actor, point&& posDiff)
	{
		if (canMove(actor, std::forward<point>(posDiff))) {
			actor->movePosBy(std::forward<point>(posDiff));
			if (auto* tile{ getTileAt(actor->pos) }; tile != nullptr)
				tile->effect(actor);
		}
	}
	void moveActor(ActorBase* actor, const position& x, const position& y)
	{
		return moveActor(actor, std::move(point{ x, y }));
	}

	void movePlayer(const point& posDiff) { moveActor(&player, posDiff); }
	void movePlayer(const position& x, const position& y) { moveActor(&player, x, y); }
#	pragma endregion Movement

	Faction& getFaction(const int& factionID) const
	{
		return GameConfig.getFactionFromID(factionID);
	}
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

	/**
	 * @brief	Performs an action for all npcs in the list.
	 *\n		This function should be called by the game loop.
	 */
	void PerformActionAllNPCs()
	{
		// get the player position
		const auto& playerPos{ player.pos };
		for (auto it{ npcs.begin() }; it != npcs.end(); ) {
			if (auto* npc{ it->get() }; npc != nullptr) {
				if (npc->isDead()) {
					it->reset();
					it = removeNPC(it);
					continue; ///< required!
				}

				point dir{ 0, 0 };
				switch (rng.get(0, 1)) {
				case 0:
					dir.x += rng.get(0, 1) == 1 ? 1 : -1;
					break;
				case 1:
					dir.y += rng.get(0, 1) == 1 ? 1 : -1;
					break;
				}

				moveActor(npc, dir);
			}
			else it = removeNPC(it);
			if (it != npcs.end())
				++it;
		}
	}
};
