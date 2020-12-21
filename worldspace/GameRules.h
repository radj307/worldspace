#pragma once
#include <chrono>
#include "actor.h"
#include "INI.hpp"

/**
 * struct GameRules
 * Gamespace requires an instance of GameRules to construct, these are generic settings that may be used by a game to allow user-customization.
 */
struct GameRules final {
private:
	/**
	 * CAN_LEVEL_UP(int, int)
	 * @brief Checks if an actor can level up or not depending on their current stats.
	 * @param level	 - Actor's current level
	 * @param kills	 - Actor's current kill count
	 * @returns bool - ( true = Actor can level up ) ( false = Actor can't level up yet )
	 */
	[[nodiscard]] constexpr bool CAN_LEVEL_UP(const int level, const int kills) const { return kills >= _level_up_kills * (level * _level_up_mult); }
	
public:
#pragma region WORLDSPACE
	// CELL / WORLD
	bool
		_walls_always_visible{ true },		// When true, wall tiles are always visible to the player.
		_override_known_tiles{ false },		// When true, the player can always see all tiles. Disables dark mode.
		_dark_mode{ true };					// When true, the player can only see the area around them.
	std::string _world_import_file{};		// The game will attempt to load cell from this file. If blank, generates a new cell.
	Coord _cellSize{ 30, 30 };				// If no filename is set, this is the size of the generated cell

	// TRAPS
	int _trap_dmg{ 20 };					// the amount of health an actor loses when they step on a trap
	const bool _trap_percentage{ true };	// whether the _trap_dmg amount is static, or a percentage of max health
#pragma endregion WORLDSPACE
#pragma region ATTACKS
	// ATTACKS
	int	_attack_cost_stamina{ 15 };	// The amount of stamina used when attacking. This is a static value.
	float
		_attack_block_chance{ 35.0f }, // 1 in (this) chance of an actor blocking a full-attack.
		_attack_miss_chance_full{ 11.0f }, // chance of an actor missing when their stamina is high enough
		_attack_miss_chance_drained{ 35.0f }; // chance of an actor missing with low stamina
#pragma endregion ATTACKS
#pragma region ACTORS
	// PLAYER
	bool _player_godmode{ false };			// When true, Player cannot be attacked
	ActorTemplate _player_template{ "Player", ActorStats(1, 120, 120, 45, 4), '$', Color::f_green }; // Player template

	// GENERIC NPC
	std::vector<FACTION>
		_enemy_hostile_to = { FACTION::PLAYER, /*FACTION::NEUTRAL*/ }, // Which factions are enemies hostile to by default.
		_neutral_hostile_to = { FACTION::NONE }; // Which factions are neutrals hostile to by default. This changes if NPC is attacked.
	float
		_npc_move_chance{ 6 },				// The chance an NPC will move when idle (1 in (this) chance)
		_npc_move_chance_aggro{ 6 };		// The chance an NPC will move when aggravated ((this) to 1 chance)
	int _npc_vis_mod_aggro{ 1 };			// This value is added to an NPC's sight range when chasing target
	
	// ENEMIES
	int
		_enemy_count{ 20 },					// how many enemies are present when the game starts
		_enemy_aggro_distance{ 3 };			// Determines from how far away enemies notice the player and become aggressive
	// ActorStats((Level), (Health), (Stamina), (Damage), (Visibility)), (Character), (Color), (Hostile factions), (Max aggression val in move cycles), (Chance to spawn))
	// Chance to spawn is calculated starting at the end of the list, so the first template should have a chance of 100
	std::vector<ActorTemplate> _enemy_template{		// Enemy templates, aka default values for enemy types.
		{ "Bandit",	 ActorStats(1, 40, 100, 15, _enemy_aggro_distance + 1), 'Y', Color::f_yellow, _enemy_hostile_to, 30, 100.0f },	// Level 1 enemy
		{ "Marauder",ActorStats(2, 40, 90, 13, _enemy_aggro_distance + 1), 'T', Color::f_red, _enemy_hostile_to, 20, 45.0f },	// Level 2 enemy
		{ "Reaver",	 ActorStats(3, 60, 90, 30, _enemy_aggro_distance), 'T', Color::f_magenta, _enemy_hostile_to, 20, 20.0f },// Level 3 enemy
		{ "Reaper",	 ActorStats(4, 60, 100, 30, _enemy_aggro_distance), 'M', Color::f_magenta, _enemy_hostile_to, 30, 2.0f },// Level 4 enemy
	};

	// NEUTRALS
	int	_neutral_count{ 12 };				// how many neutrals are present when the game starts
	std::vector<ActorTemplate> _neutral_template{	// Neutral templates, aka default values for neutral types
		{ "Chicken",ActorStats(1, 30, 30, 5, 5), '`', Color::f_cyan, _neutral_hostile_to, 100, 100.0f },
		{ "Sheep",	ActorStats(2, 30, 30, 5, 4), '@', Color::f_cyan, _neutral_hostile_to, 50, 45.0f },
		{ "Cow",	ActorStats(3, 30, 30, 5, 4), '%', Color::f_blue, _neutral_hostile_to, 35, 20.0f },
	};
#pragma endregion ACTORS
#pragma region ACTOR_EFFECTS
	// PASSIVE EFFECTS
	std::chrono::seconds _regen_timer{ 2 };
	int
		_regen_health{ 5 },					// Amount of health regenerated every second
		_regen_stamina{ 10 };				// Amount of stamina regenerated every second

	// LEVELS
	int
		_level_up_kills{ 2 },				// Kills required for an actor to level up. This value is arbitrary.
		_level_up_mult{ 2 },				// Multiplies the level_up_kills threshold for every level
		_level_up_restore_percent{ 50 };	// How much health/stamina is regenerated when the player levels up ( 0 - 100 )

	/**
	 * canLevelUp(ActorBase*)
	 * @brief Checks if a given actor can level up, based on their current stats.
	 * @param actor	 - Pointer to an actor
	 * @returns bool - ( true = can level up ) ( false = cannot level up )
	 */
	bool canLevelUp(ActorBase* actor) const
	{
		if ( actor != nullptr && CAN_LEVEL_UP(actor->getLevel(), actor->getKills()) )
			return true;
		return false;
	}
#pragma endregion ACTOR_EFFECTS
#pragma region FLARE
	unsigned short
		_level_up_flare_time{ 6 };			// How many frames to flare when the player levels up. Must be a multiple of 2
	unsigned int // Percentage of remaining enemies to trigger finale. (0 to disable.)
		_challenge_final_trigger_percent{ 25 };
	bool		 // if true, neutral NPCs will also attack the player during the finale
		_challenge_neutral_is_hostile{ false };
#pragma endregion FLARE

	/**
	 * GameRules(GLOBAL&)
	 * @brief Construct a GameRules instance from an INI file.
	 * @param cfg	- Ref to an INI instance.
	 */
	explicit GameRules(INI& cfg) :
		_walls_always_visible(cfg.get<bool>("world", "showAllWalls", INI::stoi)),
		_override_known_tiles(cfg.get<bool>("world", "showAllTiles", INI::stoi)),
		_dark_mode(cfg.get<bool>("world", "fogOfWar", INI::stoi)),
		_world_import_file(cfg.get("world", "importFromFile")),
		_cellSize({cfg.get<long>("world", "sizeH", INI::stol), cfg.get<long>("world", "sizeV", INI::stol) }),
		_trap_dmg(cfg.get<int>("world", "trapDamage", INI::stoi)),
		_trap_percentage(cfg.get<bool>("world", "trapDamageIsPercentage", INI::stoi)),
		_attack_cost_stamina(cfg.get<int>("actors", "attackCostStamina", INI::stoi)),
		_attack_block_chance(cfg.get<float>("actors", "attackBlockChance", INI::stof)),
		_attack_miss_chance_full(cfg.get<float>("actors", "attackMissChanceFull", INI::stof)),
		_attack_miss_chance_drained(cfg.get<float>("actors", "attackMissChanceDrained", INI::stof)),
		_player_godmode(cfg.get<bool>("player", "godmode", INI::stoi)),
		_npc_move_chance(cfg.get<float>("actors", "npcMoveChance", INI::stof)),
		_npc_move_chance_aggro(cfg.get<float>("actors", "npcMoveChanceAggro", INI::stof)),
		_npc_vis_mod_aggro(cfg.get<int>("actors", "npcVisModAggro", INI::stoi)),
		_enemy_count(cfg.get<int>("enemy", "count", INI::stoi)),
		_enemy_aggro_distance(cfg.get<int>("enemy", "aggroDistance", INI::stoi)),
		_neutral_count(cfg.get<int>("neutral", "count", INI::stoi)),
		_regen_timer(cfg.get<int>("actors", "regen_time", INI::stoi)),
		_regen_health(cfg.get<int>("actors", "regen_health", INI::stoi)),
		_regen_stamina(cfg.get<int>("actors", "regen_stamina", INI::stoi)),
		_level_up_kills(cfg.get<int>("player", "levelKillThreshold", INI::stoi)),
		_level_up_mult(cfg.get<int>("player", "levelKillMult", INI::stoi)),
		_level_up_restore_percent(cfg.get<int>("player", "levelRestorePercent", INI::stoi))
	{
		// Set player stats
		if ( !cfg.get("player", "name").empty() )		// Check name
			_player_template._name = cfg.get("player", "name");
		if ( !cfg.get("player", "health").empty() )	// Check health
			_player_template._stats.setMaxHealth(cfg.get<int>("player", "health", INI::stoi));
		if ( !cfg.get("player", "stamina").empty() )	// Check stamina
			_player_template._stats.setMaxStamina(cfg.get<int>("player", "stamina", INI::stoi));
		if ( !cfg.get("player", "damage").empty() )	// Check damage
			_player_template._stats.setMaxDamage(cfg.get<int>("player", "damage", INI::stoi));
	}

	// Default constructor
	GameRules() = default;
};