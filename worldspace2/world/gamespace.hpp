#pragma once
#include "../actors/ActorBase.hpp"
#include "matrix.hpp"

#include <math.hpp>
#include <xRand.hpp>

#include <optional>
#include <map>

using size = point;

static struct {
	size gridSize{ 30, 30 };
	GeneratorSettings generatorConfig{};

	point minPos{ point{ 0, 0 } + static_cast<int>(generatorConfig.wall_always_on_edge) };
	point maxPos{ gridSize - minPos };

	static void setGridSize(const size& newGridSize)
	{
		GameConfig.gridSize = newGridSize;
		GameConfig.minPos = point{ 0, 0 } + static_cast<int>(GameConfig.generatorConfig.wall_always_on_edge);
		GameConfig.maxPos = GameConfig.gridSize - GameConfig.minPos;
	}

	std::array<Faction, 5> factions{
		Faction{ NULL_FACTION_ID },
		Faction{ 0, { 1 } },
		Faction{ 1, { 0, 2 } },
		Faction{ 2 },
		Faction{ 3 }
	};

	Faction& getFactionFromID(const int& id)
	{
		for (int i{ 1 }; i < factions.size(); ++i)
			if (auto& fac{ factions[i] }; fac == id)
				return fac;
		return factions[0];
	}

	Faction& playerFaction{ factions[1] };
	Faction& enemyFaction{ factions[2] };
	Faction& neutralFaction{ factions[3] };
	Faction& passiveFaction{ factions[4] };

	std::vector<ActorTemplate> npc_templates{
		ActorTemplate{
			DisplayableBase{ '*', color::setcolor::cyan },
			passiveFaction.operator int(),
			1u,
			"Chicken"s,
			StatFloat(30.0f),
			StatFloat(50.0f),
			StatFloat(15.0f),
			StatFloat(0.0f),
			StatFloat{ 10.0f, 0.0f },
			StatFloat{ 0.0f, 0.0f }
	},
		ActorTemplate{
			DisplayableBase{ '*', color::setcolor::cyan },
			neutralFaction,
			2,
			"Ram",
			80.0f,
			150.0f,
			20.0f,
			5.0f,
			StatFloat{ 40.0f, 0.0f },
			StatFloat{ 0.0f, 0.0f }
	}
	};
	std::vector<ActorTemplate> enemy_templates{
		ActorTemplate{
			DisplayableBase{ '?', color::setcolor::red },
			enemyFaction,
			1,
			"Bandit"s,
			100.0f,
			80.0f,
			10.0f,
			2.5f,
			StatFloat{ 50.0f, 0.0f },
			StatFloat{ 100.0f, 0.0f }
	},
		ActorTemplate{
			DisplayableBase{ '!', color::setcolor::magenta },
			enemyFaction,
			2,
			"Marauder",
			110.0f,
			100.0f,
			20.0f,
			5.0f,
			StatFloat{ 100.0f, 0.0f },
			StatFloat{ 100.0f, 0.0f }
	},
		ActorTemplate{
			DisplayableBase{ '%', color::setcolor{ color::rgb_to_sgr(1.0f, 0.2f, 0.01f) } },
			enemyFaction,
			3,
			"Reaver",
			120.0f,
			190.0f,
			19.6f,
			22.2f,
			StatFloat{ 100.0f, 0.0f },
			StatFloat{ 100.0f, 0.0f }
	}
	};

	ActorTemplate player_template{
		DisplayableBase{ '$', color::setcolor::green },
		playerFaction,
		1,
		"Player",
		100.0f,
		100.0f,
		25.0f,
		25.0f,
		std::nullopt,
		std::nullopt
	};

	size_t generate_npc_count{ 10ull };
	size_t generate_enemy_count{ 20ull };
	double npcDistribRate{ 1.0 }; ///< @brief	This is the value for (lambda) in an exponential distribution function. It is used to select actor templates less often when placed at high (relative) indexes in the ActorTemplate vectors above. The lower the value, the closer to a straight line. The higher the value, the more curved. See this image for a graphic: ( https://en.wikipedia.org/wiki/Exponential_distribution#/media/File:Exponential_cdf.svg ).
} GameConfig;

inline static bool isWithinBounds(const point& pos)
{
	if (pos.within(GameConfig.minPos, GameConfig.maxPos))
		return true;
	return false;
}

struct gamespace {
	rng::Random rng;
	matrix grid;
	Player player;
	std::vector<std::unique_ptr<NPC>> npcs;

	gamespace() : grid{ rng, GameConfig.gridSize, GameConfig.generatorConfig }, player{ [this]() {
		const auto& cont{ getAllValidSpawnTiles<floortile>() };
		return cont.at(rng.get(0, cont.size()));
	}(), GameConfig.player_template }
	{
		generate<NPC>(GameConfig.generate_npc_count, GameConfig.npc_templates);
		generate<NPC>(GameConfig.generate_enemy_count, GameConfig.enemy_templates);
	}

	template<std::derived_from<tile>... ValidSpawnTiles>
	std::vector<point> getAllValidSpawnTiles()
	{
		std::vector<point> vec;
		vec.reserve(grid.Size);

		size_t i{ 0ull };
		for (const auto& it : grid) {
			if (auto* tile{ it.get() }; var::variadic_or(typeid(*tile) == typeid(ValidSpawnTiles)...))
				if (point pos{ grid.from1D(i) }; getActorAt(pos) == nullptr)
					vec.emplace_back(std::move(pos));
			++i;
		}

		vec.shrink_to_fit();
		return vec;
	}
	std::vector<point> getAllValidSpawnTiles()
	{
		return getAllValidSpawnTiles<floortile>();
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

	bool canMove(ActorBase* actor, const point& posDiff) noexcept(false)
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

	bool PerformActionNPC(NPC* npc)
	{
		if (npc == nullptr)
			throw make_exception("gamespace::PerformActionNPC() failed:  Received nullptr!");

		if (auto* target{ npc->getTarget() }; target != nullptr) {

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

	/**
	 * @brief	Performs an action for all npcs in the list.
	 *\n		This function should be called by the game loop.
	 */
	void PerformActionAllNPCs()
	{
		for (auto it{ npcs.begin() }; it != npcs.end(); ) {
			if (auto* npc{ it->get() }; npc != nullptr) {
				if (npc->isDead()) {
					it->reset();
					it = removeNPC(it);
					continue; ///< required!
				}
				else PerformActionNPC(npc);
			}
			else it = removeNPC(it);
			if (it != npcs.end())
				++it;
		}
	}
};
