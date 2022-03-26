#pragma once
#include "../actors/ActorBase.hpp"
#include "matrix.hpp"
#include "../GameConfig.h"
#include "../actors/Projectile.hpp"

#include <math.hpp>
#include <xRand.hpp>

#include <optional>
#include <map>
#include <queue>

struct flare {
	int framesRemaining;

	constexpr flare(const int& frameLength = 0) : framesRemaining{ frameLength } {}
	virtual ~flare() = default;

	constexpr bool isOver() const { return framesRemaining <= 0; }

	virtual std::optional<color::setcolor> getFlareAt(const position&, const position&) const = 0;
	std::optional<color::setcolor> getFlareAt(const point& p) const { return getFlareAt(p.x, p.y); }
};

struct edgeflare : flare {
	color::setcolor color;
	bounds size;
	edgeflare(const int& length, color::setcolor&& color, bounds&& gridSize) : flare(length), color{ std::move(color) }, size{ std::move(gridSize) } {}
	edgeflare(const int& length, const color::setcolor& color, const bounds& gridSize) : flare(length), color{ color }, size{ gridSize } {}
	edgeflare(const int& length, const color::setcolor& color, const point& gridMax) : flare(length), color{ color }, size{ point{ 0, 0 }, gridMax } {}
	virtual std::optional<color::setcolor> getFlareAt(const position& x, const position& y) const override
	{
		const auto& [min, max] {size};
		return (x == min.x || x == max.x - 1 || y == min.y || y == max.y - 1) ? color : static_cast<std::optional<color::setcolor>>(std::nullopt);
	}
};

template<size_t I, typename T, size_t Count = 0ull, typename... Ts>
size_t get_index_of(size_t& count)
{
	if constexpr (std::same_as<T, decltype(std::get<I>(std::declval<std::tuple<Ts...>>()))>) {
		if (count >= Count)
			return count;
		else ++count;
		if constexpr (I + 1 < sizeof...(Ts))
			return get_index_of<I + 1, T, Count, Ts...>(count);
	}
	return count;
}


template<typename T>
struct Container;

template<>
struct Container<int> {
	std::vector<int> vec;

	Container(std::vector<int>&& vec) : vec{ std::move(vec) } {}
	template<var::same_or_convertible<int>... Ts>
	Container(Ts&&... integers) : vec{ static_cast<int>(std::forward<Ts>(integers))... } {}
};

template<>
struct Container<float> {
	float value;
	Container(float&& value) : value{ std::move(value) } {}
	Container(const float& value) : value{ value } {}
};


struct gamespace {
	rng::Random rng;
	matrix grid;
	bounds boundaries;
	Player player;
	std::queue<std::unique_ptr<flare>> flares;

	using NPCContainer = std::vector<std::unique_ptr<NPC>>;
	using ProjContainer = std::vector<std::unique_ptr<Projectile>>;

	NPCContainer npcs;
	ProjContainer projectiles;

	gamespace() : grid{ rng, GameConfig.gridSize, GameConfig.generatorConfig }, boundaries{ getPlayableBounds() }, player{ [this]() {
		const auto& cont{ getAllValidSpawnTiles<floortile>(false) };
		return cont.at(rng.get(0, cont.size()));
	}(), GameConfig.player_template }
	{
		generate<NPC>(GameConfig.generate_npc_count, GameConfig.npc_templates);
		generate<Enemy>(GameConfig.generate_enemy_count, GameConfig.enemy_templates);

		addFlare<edgeflare>(6, color::setcolor{ color::green, color::Layer::B }, GameConfig.gridSize);
	}

	template<std::derived_from<flare> T, typename... Args>
	void addFlare(Args&&... args)
	{
		flares.push(std::make_unique<T>(std::forward<Args>(args)...));
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

	Projectile* getProjectileAt(const point& pos) const
	{
		for (auto it{ projectiles.begin() }; it != projectiles.end(); ++it)
			if (auto* proj{ it->get() }; proj->pos == pos)
				return proj;
		return nullptr;
	}
	Projectile* getProjectileAt(const position& x, const position& y) const
	{
		return getProjectileAt(point{ x, y });
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
		if (!newPos.within(boundaries)) // if the pos is out of bounds, return false early
			return false;
		if (auto* tile{ getTileAt(newPos) }; tile != nullptr && typeid(*tile) != typeid(walltile)) {
			// check for actors
			if (const auto& other{ getActorAt(newPos) }; other != nullptr) {
				// check if the actor is hostile towards the target
				if (typeid(*actor) == typeid(Player) || GameConfig.getFactionFromID(actor->factionID).isHostileTo(other->factionID)) {
					return other->applyDamageFrom(actor); // return true if other actor died
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

	bool fireProjectile(ActorBase* actor, const point& direction)
	{
		const auto& origin{ actor->pos + direction };
		if (getProjectileAt(origin) != nullptr)
			return false;
		// check if the projectile spawn location currently has an actor located there
		if (auto* o{ getActorAt(origin) }; o != nullptr) {
			const auto& fID{ actor->factionID };
			if (auto& oFaction{ getFaction(o->factionID) }; !oFaction.isHostileTo(fID))
				oFaction.addHostile(fID);
			// create a temporary projectile and attack the actor
			Projectile tmp{ actor, origin, direction, actor->damage, true };
			o->applyDamage(tmp.damage, tmp.piercing);
			return true;
		}
		else {
			projectiles.emplace_back(std::make_unique<Projectile>(actor, origin, direction, actor->damage * 2, true));
			return true;
		}
		return false;
	}
	bool playerFireProjectile(const point& direction)
	{
		return fireProjectile((ActorBase*)&player, direction);
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
	 * @returns		NPCContainer::iterator
	 */
	NPCContainer::iterator removeNPC(const NPCContainer::const_iterator& it)
	{
		if (npcs.empty())
			throw make_exception("gamespace::removeNPC() failed:  Cannot remove elements from an empty list!");
		return npcs.erase(it, it + 1);
	}
	/**
	 * @brief		Remove a projectile from the list using a given iterator.
	 *\n			If you're calling this from a for loop, make sure to set the given
	 *\n			iterator to the return value of this function to prevent a hanging pointer.
	 * @param it	An iterator pointing to the target projectile.
	 * @returns		ProjContainer::iterator
	 */
	ProjContainer::iterator removeProjectile(const ProjContainer::const_iterator& it)
	{
		if (projectiles.empty())
			throw make_exception("gamespace::removeProjectile() failed:  Cannot remove elements from an empty list!");
		return projectiles.erase(it, it + 1);
	}

	point pathFind(const point& start, const point& target)
	{
		const auto& movable{ [this](const point& p) {
			if (p.within(boundaries))
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

	void applyToAll(const std::function<void(ActorBase*)>& function)
	{
		// apply to player
		function((ActorBase*)&player);
		// apply to npcs
		for (const auto& it : npcs)
			function(it.get());
	}

	void PerformPeriodicRegen()
	{
		applyToAll([](ActorBase* actor) {
			actor->health += GameConfig.regenHealth;
			actor->stamina += GameConfig.regenStamina;
		});
	}

	/**
	 * @brief	Count the number of actors that are of a specified type.
	 * @returns	size_t
	 */
	template<std::derived_from<ActorBase>... Types>
	size_t countNPCsWithType() const
	{
		size_t count{ 0ull };
		for (const auto& it : npcs)
			if (auto* npc{ it.get() }; npc != nullptr)
				if (const auto& myType{ typeid(*npc) }; var::variadic_or(myType == typeid(Types)...))
					++count;
		return count;
	}
	template<std::same_as<ID>... Ids> requires var::at_least_one<Ids...>
	size_t countNPCsWithFaction(Ids&&... ids) const
	{
		size_t count{ 0ull };
		for (const auto& it : npcs)
			if (auto* npc{ it.get() }; npc != nullptr)
				if (const auto& myFaction{ npc->factionID }; var::variadic_or(myFaction == ids...))
					++count;
		return count;
	}

	point getRandomDir()
	{
		point dir{ rng.get(-1, 1), rng.get(-1, 1) };
		if (dir.x != 0 && dir.y != 0) {
			if (rng.get(0, 1) == 0)
				dir.x = 0;
			else
				dir.y = 0;
		}
		return dir;
	}

	/**
	 * @brief		Perform a turn for one NPC.
	 * @param npc	Pointer to an NPC.
	 * @returns		bool
	 */
	bool PerformActionNPC(NPC* npc) noexcept(false)
	{
		if (npc == nullptr)
			throw make_exception("gamespace::PerformActionNPC() failed:  Received nullptr!");

		const auto& myFaction{ getFaction(npc->factionID) };

		const auto& findNearbyTarget{ [&npc, &myFaction, this]() {
			//auto* nearby{ findNearbyActor(npc->aggressionRange, npc->pos, [&myFaction](ActorBase* actor) {
//				return myFaction.isHostileTo(actor->factionID);
			//}) };
//			if (nearby != nullptr)
//				npc->target = nearby->pos;
		} };

		// NPC has a target set
		if (npc->target.has_value()) {
			const point targetPos{ npc->target.value() };

			// target isn't valid, search nearby for a valid target
			if (auto* target{ getActorAt(targetPos) }; target == nullptr || !myFaction.isHostileTo(target->factionID))
				findNearbyTarget();
		}
		else findNearbyTarget();

		// npc found a target
		if (npc->target.has_value()) {
			const auto& targetPos{ npc->target.value() };
			moveActor(npc, pathFind(npc, npc->isAfraid() ? -targetPos : targetPos));
		}

		// npc still doesn't have a target
		else if (rng.get(0.0f, 100.0f) <= GameConfig.npcIdleMoveChance)
			moveActor(npc, getRandomDir());

		return npc->isDead();
	}

	/**
	 * @brief	Performs an action for all npcs in the list.
	 *\n		This function should be called by the game loop.
	 */
	void PerformActionAllNPCs() noexcept(false)
	{
		// handle NPCs
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

	/**
	 * @brief	Processes all projectiles currently
	 */
	void ProcessProjectileActions() noexcept(false)
	{
		if (!projectiles.empty()) {
			for (auto it{ projectiles.begin() }; it < projectiles.end();) {
				if (auto* proj{ it->get() }; proj != nullptr) {
					if (const auto& nextPos{ proj->nextPos() }; nextPos.within(boundaries)) {
						if (auto* tile{ getTileAt(nextPos) }; tile != nullptr) {
							if (!tileAllowsMovement(tile))
								it = removeProjectile(it);
							else if (auto* actor{ getActorAt(nextPos) }; actor != nullptr) {
								actor->applyDamage(proj->damage, proj->piercing);
								it = removeProjectile(it);
							}
							else proj->move();
							if (std::distance(it, projectiles.end()) > 0) {
								++it;
								continue;
							}
							else break;
						}
					}
				}
				if (!projectiles.empty())
					it = removeProjectile(it);
			}
		}
	}
};
