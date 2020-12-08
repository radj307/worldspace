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
#include "GameState.h"
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

	// Actor list
	std::vector<std::shared_ptr<ActorBase>> _actors;
	int _enemy_count{}, _neutral_count{};
	std::vector<std::shared_ptr<ItemStaticBase>> _items;
	
	// player character
//	Player _player;
	// generic enemies
//	std::vector<Enemy> _hostile;
	// neutral actors
//	std::vector<Neutral> _neutral;

	// Static Items - Health
//	std::vector<ItemStaticHealth> _item_static_health;
	// Static Items - Stamina
//	std::vector<ItemStaticStamina> _item_static_stamina;

	// Declare Flare instances
	std::unique_ptr<Flare> _flare;	// Pointer to one of the above instances, this should only be accessed through the flare functions.

	template<typename FlareType> bool changeFlare(FlareType flare);
	[[nodiscard]] Coord findValidSpawn(bool isPlayer = false, bool checkForItems = true);
	template<typename NPC_Type> void populate_actor_vec(unsigned int count, std::vector<ActorTemplate>& templates);
	template<typename ItemType> void populate_item_vec(unsigned int count, int amount, std::vector<FACTION> factionLock);
	void apply_to_all(void (Gamespace::*func)(const std::shared_ptr<ActorBase>&));
	void apply_to_npc(void (Gamespace::*func)(const std::shared_ptr<NPC>&));
	void apply_to_npc(bool (Gamespace::*func)(const std::shared_ptr<NPC>&));
	[[nodiscard]] std::vector<std::shared_ptr<NPC>> get_all_npc();
	[[nodiscard]] std::vector<std::shared_ptr<ItemStaticBase>> get_all_static_items();
	void regen(const std::shared_ptr<ActorBase>& actor);
	static void regen(const std::shared_ptr<ActorBase>& actor, int percent);
	void level_up(const std::shared_ptr<ActorBase>& a);
	[[nodiscard]] char getRandomDir();
	[[nodiscard]] bool canMove(const Coord& pos);
	[[nodiscard]] bool canMove(int posX, int posY);
	[[nodiscard]] bool checkMove(const Coord& pos, FACTION myFac);
	bool move(const std::shared_ptr<ActorBase>& actor, char dir);
	[[nodiscard]] bool moveNPC(const std::shared_ptr<NPC>& npc, bool noFear = false);
	int attack(const std::shared_ptr<ActorBase>& attacker, const std::shared_ptr<ActorBase>& target);
	bool actionNPC(const std::shared_ptr<NPC>& npc);
	[[nodiscard]] constexpr bool trigger_final_challenge(const unsigned int remainingEnemies) const { return remainingEnemies <= _ruleset._enemy_count * _ruleset._challenge_final_trigger_percent / 100; }

public:
	std::vector<std::vector<bool>> _ACTOR_MAP;
	// CONSTRUCTOR
	explicit Gamespace(GameRules& ruleset);
	void actionAllNPC();
	void actionPlayer(char key);
	void apply_level_ups();
	void apply_passive();
	void cleanupDead();
	[[nodiscard]] std::shared_ptr<ActorBase> getNearbyActor(const Coord& pos, int visRange);
	[[nodiscard]] std::shared_ptr<ActorBase> getActorAt(const Coord& pos);
	[[nodiscard]] std::shared_ptr<ActorBase> getActorAt(int posX, int posY);
	[[nodiscard]] std::shared_ptr<ItemStaticBase> getItemAt(const Coord& pos);
	[[nodiscard]] std::shared_ptr<ItemStaticBase> getItemAt(int posX, int posY);
	[[nodiscard]] std::shared_ptr<Player> getPlayer();
	[[nodiscard]] Tile& getTile(const Coord& pos);
	[[nodiscard]] Tile& getTile(int x, int y);
	[[nodiscard]] Cell& getCell();
	[[nodiscard]] Coord getCellSize() const;
	[[nodiscard]] GameRules& getRuleset() const;
	std::unique_ptr<Flare>& getFlare();
	void resetFlare();
	
	// Contains information about the game outcome.
	GameState _game_state;
};