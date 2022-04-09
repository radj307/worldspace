#pragma once
#include "../actors/ActorBase.hpp"
#include "matrix.hpp"
#include "../GameConfig.h"
#include "../actors/Projectile.hpp"

#include <math.hpp>
#include <xRand.hpp>

#include <optional>
#include <map>
#include <set>
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
				if (typeid(*actor) == typeid(Player) || getFaction(actor->factionID).isHostileTo(other->factionID)) {
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

	/**
	 * @brief				Make the specified actor fire a projectile in a given direction.
	 * @param actor			The owner of the projectile.
	 * @param direction		The direction of travel of the spawned projectile.
	 * @returns				bool
	 *						True when a projectile was successfully fired, even if it wasn't added to the projectiles list because there was an actor at the origin.
	 */
	bool fireProjectile(ActorBase* actor, const point& direction)
	{
		const point& myPos{ actor->pos }, & origin{ myPos + direction };

		// don't spawn the projectile if one already exists at the origin
		if (getProjectileAt(origin) != nullptr)
			return false;

		if (auto* tile{ getTileAt(origin) }; tile != nullptr) { // throw an exception if the getTileAt function failed & didn't throw one itself.
			// check if the origin point is a valid movable tile
			if (tileAllowsMovement(tile)) {
				// Create a projectile
				Projectile proj{ actor, origin, direction, actor->damage, true };

				// check if there is an actor at the origin tile
				if (auto* actorAtPos{ getActorAt(origin) }; actorAtPos != nullptr) {
					auto& myFaction{ getFaction(actor->factionID) };

					// if the unlucky winner of a bullet to the face isn't already hostile, make them hostile
					if (auto& theirFaction{ getFaction(actorAtPos->factionID) }; !theirFaction.isHostileTo(myFaction))
						theirFaction.addHostile(myFaction);

					// apply the projectile's damage directly to the unlucky winner
					actorAtPos->applyDamage(proj.damage, proj.piercing);

					// don't add the projectile to the 'in-flight' list because it already hit someone
					return true;
				}
				// else there is not an actor at the origin tile, add the projectile to the 'in-flight' list.
				projectiles.emplace_back(std::make_unique<Projectile>(std::move(proj)));
				return true;
			} // else origin isn't a movable tile, don't spawn the projectile
			return false;
		}
		else throw make_exception("fireProjectile() failed:  Out-of-range origin position ( ", origin.x, ", ", origin.y, " )!");
	}
	/**
	 * @brief				Make the player fire a projectile in a given direction.
	 * @param direction		The direction of travel of the spawned projectile.
	 * @returns				bool
	 *						True when a projectile was successfully fired, even if it wasn't added to the projectiles list because there was an actor at the origin.
	 */
	bool playerFireProjectile(const point& direction)
	{
		return fireProjectile((ActorBase*)&player, direction);
	}

	/**
	 * @brief			Retrieve the faction object associated with a given faction ID number.
	 *\n				This calls `GameConfig.getFactionFromID()`, which should not be directly called anywhere else within the gamespace object.
	 * @param factionID	The ID number of a faction.
	 * @returns			Faction
	 */
	Faction& getFaction(const int& factionID) { return GameConfig.getFactionFromID(factionID); }

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

		return diffNormal;
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

	ActorBase* findNearbyActor(const point& pos, const position& radius, const std::function<bool(ActorBase*)>& pred, const bool& include_pos = false)
	{
		const auto& checkPos{ [&pred, this](const point& p) {
			auto* atPos{ getActorAt(p) };
			return (atPos != nullptr && !atPos->isDead()) ? atPos : nullptr;
		} };

		if (include_pos)
			if (auto* atPos{ checkPos(pos) }; atPos != nullptr)
				return atPos;

		for (position i{ 1 }; i <= radius; ++i) {
			for (position startY{ pos.y - i }, endY{ pos.y + i }, y{ startY }; y <= endY; ++y) {
				const position& startX{ pos.x - i }, endX{ pos.x + i };
				if (y == startY || y == endY) { // top/bottom row, iterate through all columns
					for (position x{ startX }; x <= endX; ++x) {
						if (const point& here{ x, y }; here != pos && here.within(boundaries) && here.withinCircle(radius, pos)) {
							if (auto* atPos{ getActorAt(here) }; atPos != nullptr && pred(atPos))
								return atPos;
						}
					}
				} // middle row, check only the first and last column (since all interior cells have already been checked)
				else if (auto* atPos{ checkPos(point(startX, y)) }; atPos != nullptr)
					return atPos;
				else if (atPos = checkPos(point(endX, y)); atPos != nullptr)
					return atPos;
			}
		}

		return nullptr;
	}

	/**
	 * @brief		Perform a turn for one NPC.
	 * @param npc	Pointer to an NPC.
	 */
	void PerformActionNPC(NPC* npc) noexcept(false)
	{
		if (npc == nullptr)
			throw make_exception("gamespace::PerformActionNPC() failed:  Received nullptr!");

		const auto& myFaction{ getFaction(npc->factionID) };

		// NPC has a target set:
		if (auto* tgt{ npc->getTarget() }; tgt != nullptr) {
			if (isValidFaction(tgt->factionID) && !tgt->isDead()) {
				if (tgt->pos.withinCircle(npc->aggressionRange, npc->pos))
					npc->aggression += 10.0f;
				else npc->aggression -= 15.0f;
				if (npc->aggression <= 0.0f)
					npc->unsetTarget();
				else moveActor(npc, pathFind(npc, npc->isAfraid() ? -tgt->pos : tgt->pos));
				return;
			}
			else npc->unsetTarget();
		}

		if (auto* tgt{ findNearbyActor(npc->pos, npc->aggressionRange, [&myFaction](ActorBase* actor) -> bool {
			return myFaction.isHostileTo(actor->factionID);
			}) }; tgt != nullptr) {
			npc->setTarget(tgt);
			moveActor(npc, pathFind(npc, tgt->pos));
			return;
		}

		// npc still doesn't have a target
		if (rng.get(0.0f, 100.0f) <= GameConfig.npcIdleMoveChance)
			moveActor(npc, getRandomDir());
	}

	/**
	 * @brief	Performs an action for all npcs in the list.
	 *\n		This function should be called by the game loop.
	 */
	void PerformActionAllNPCs() noexcept(false)
	{
		if (npcs.empty())
			return;
		for (auto it{ npcs.begin() }; it != npcs.end(); ++it) {
			auto* npc{ it->get() };
			if (npc == nullptr || npc->isDead()) {
				it = npcs.erase(it);
				if (it == npcs.end()) break;
			}
			else PerformActionNPC(npc);
		}
	}

	/**
	 * @brief	Processes all projectiles currently
	 */
	void ProcessProjectileActions() noexcept(false)
	{
		for (auto it{ projectiles.begin() }; it != projectiles.end(); ++it) {
			if (auto* proj{ it->get() }; proj != nullptr) {
				const point nextPos{ proj->nextPos() };
				if (auto* nextTile{ getTileAt(nextPos) }; nextTile != nullptr && tileAllowsMovement(nextTile)) {
					// check if there is an unlucky winner of a bullet to the face in the next position
					if (auto* actorAtPos{ getActorAt(nextPos) }; actorAtPos != nullptr) {
						actorAtPos->applyDamage(proj->damage, proj->piercing);
						// fallthrough & erase this projectile
					}
					else { // increment this projectiles position & continue
						proj->moveToNextPos();
						continue;
					}
				} // else tile is null, or doesn't allow movement
			} // else projectile is null

			// fallthrough deletes this projectile if continue wasn't called.
			it = projectiles.erase(it);

			if (it == projectiles.end()) break;
		}
	}
};
