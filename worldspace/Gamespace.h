/**
 * game.h
 * Represents the game, with a worldspace & actors.
 * Contains the Gamespace class, which is a container for cells, and actors.
 * @author radj307
 */
#pragma once
#include "actor.h"
#include "cell.h"
#include "Flare.h"
#include "GameRules.h"
#include "GameState.h"
#include "item.h"

 /**
  * @class Gamespace
  * @brief Contains all of the game-related functions required for running a game. Does not contain any display functions, use external FrameBuffer.
  */
class Gamespace final {
	// Reference to the game's ruleset
	GameRules& _ruleset;
	// worldspace cell
	Cell _world;
	// Randomization engine
	rng::tRand _rng;
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
	std::vector<Flare*> _FLARE_QUEUE{};
	FlareLevel _FLARE_DEF_LEVEL;			// Flare used when the player levels up
	FlareChallenge _FLARE_DEF_CHALLENGE;	// Flare used when the final challenge mode begins
	FlareBoss _FLARE_DEF_BOSS;

	void addFlare(Flare& newFlare);
	[[nodiscard]] Coord findValidSpawn(bool isPlayer = false, bool checkForItems = true);
	template<typename Actor> [[nodiscard]] std::vector<Actor> generate_NPCs(int count, std::vector<ActorTemplate>& templates);
	template<typename Item> [[nodiscard]] std::vector<Item> generate_items(int count, bool lockToPlayer = false);
	template<typename NPC> [[nodiscard]] NPC build_npc(ActorTemplate& actorTemplate);
	template<typename NPC> [[nodiscard]] NPC build_npc(const Coord& pos, ActorTemplate& actorTemplate);
	void spawn_boss();
	void update_state() noexcept;
	void apply_to_all(void (Gamespace::* func)(ActorBase*));
	void apply_to_npc(void (Gamespace::* func)(NPC*));
	void apply_to_npc(bool (Gamespace::* func)(NPC*));
	void regen(ActorBase* actor);
	static void regen(ActorBase* actor, int percent);
	void level_up(ActorBase* a);
	[[nodiscard]] char getRandomDir();
	[[nodiscard]] bool canMove(const Coord& pos);
	[[nodiscard]] bool canMove(int posX, int posY);
	[[nodiscard]] bool checkMove(const Coord& pos, FACTION myFac);
	void trap(ActorBase* actor, bool didMove);
	[[nodiscard]] bool move(ActorBase* actor, char dir);
	[[nodiscard]] bool moveNPC(NPC* npc, bool noFear = false);
	int attack(ActorBase* attacker, ActorBase* target);
	bool actionNPC(NPC* npc);
	[[nodiscard]] constexpr bool trigger_final_challenge(const unsigned int remainingEnemies) const { return remainingEnemies <= _ruleset._enemy_count * _ruleset._challenge_final_trigger_percent / 100; }

public:
	// CONSTRUCTOR
	explicit Gamespace(GameRules& ruleset) noexcept;

	[[nodiscard]] std::vector<ActorBase*> get_all_actors();
	[[nodiscard]] std::vector<NPC*> get_all_npc();
	[[nodiscard]] std::vector<ItemStaticBase*> get_all_static_items();
	void actionAllNPC();
	void actionPlayer(char key);
	void apply_level_ups();
	void apply_passive();
	void cleanupDead() noexcept;
	[[nodiscard]] ActorBase* getClosestActor(const Coord& pos, int visRange);
	[[nodiscard]] ActorBase* getActorAt(const Coord& pos);
	[[nodiscard]] ActorBase* getActorAt(int posX, int posY);
	[[nodiscard]] ItemStaticBase* getItemAt(const Coord& pos);
	[[nodiscard]] ItemStaticBase* getItemAt(int posX, int posY);
	[[nodiscard]] Player& getPlayer();
	[[nodiscard]] Tile* getTile(const Coord& pos);
	[[nodiscard]] Tile* getTile(int x, int y);
	[[nodiscard]] Cell& getCell();
	[[nodiscard]] Coord getCellSize() const;
	[[nodiscard]] GameRules& getRuleset() const;
	[[nodiscard]] Flare* getFlare() const;
	void resetFlare();

	// Contains information about the game outcome.
	GameState _game_state;
};