#pragma once
#include <cassert>
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
		_dark_mode{ false };					// When true, the player can only see the area around them.
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
		_enemy_aggro_distance{ 2 };			// Determines from how far away enemies notice the player and become aggressive
	// ActorStats((Level), (Health), (Stamina), (Damage), (Visibility)), (Character), (Color), (Hostile factions), (Max aggression val in move cycles), (Chance to spawn))
	// Chance to spawn is calculated starting at the end of the list, so the first template should have a chance of 100
	std::vector<ActorTemplate> _enemy_template{		// Enemy templates, aka default values for enemy types.
		{ "Bandit",	 ActorStats(1, 40, 100, 15, _enemy_aggro_distance + 1), 'Y', Color::f_yellow, _enemy_hostile_to, 30, 100.0f },	// Level 1 enemy
		{ "Marauder",ActorStats(2, 40, 90, 13, _enemy_aggro_distance + 1), 'T', Color::f_red, _enemy_hostile_to, 20, 45.0f },	// Level 2 enemy
		{ "Reaver",	 ActorStats(3, 60, 90, 30, _enemy_aggro_distance), 'T', Color::f_magenta, _enemy_hostile_to, 20, 20.0f },// Level 3 enemy
		{ "Reaper",	 ActorStats(4, 60, 100, 30, _enemy_aggro_distance), 'M', Color::f_magenta, _enemy_hostile_to, 30, 2.0f },// Level 4 enemy
	};
	// Enemy bosses
	std::vector<ActorTemplate> _enemy_boss_template{
		{ "Grim Reaper", ActorStats(10, 25, 50, 40, _cellSize._x * _cellSize._y), 'N', Color::b_magenta, _enemy_hostile_to, 100, 0.0 },
		{ "Pit Boss", ActorStats(10, 25, 50, 40, _enemy_aggro_distance + 2), 'N', Color::b_magenta, _enemy_hostile_to, 100, 0.0 },
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
	bool
		_enable_boss{ true },
		_boss_spawns_after_final{ true };
#pragma endregion FLARE

	/**
	 * buildTemplate(INI&, string&)
	 * @brief Builds an ActorTemplate from the specified INI section.
	 * @param target	- Ref of the target template instance.
	 * @param cfg		- Ref to the INI instance
	 * @param section	- INI section name containing variables
	 */
	static void buildTemplate(ActorTemplate& target, INI& cfg, const std::string& section)
	{
		if ( cfg.contains(section) ) {
			target = {
				cfg.get(section, "name"),
				ActorStats(cfg.get<int>(section, "level", conv::stoi),
				cfg.get<int>(section, "health", conv::stoi),
				cfg.get<int>(section, "stamina", conv::stoi),
				cfg.get<int>(section, "damage", conv::stoi),
				cfg.get<int>(section, "visRange", conv::stoi)),
				cfg.get(section, "char").at(0),
				Color::stoColor(cfg.get(section, "color")),
				strToFactionList(cfg.get(section, "hostileTo")),
				cfg.get<int>(section, "maxAggro", conv::stoi),
				cfg.get<float>(section, "spawnChance", conv::stof)
			};
		} // else do nothing
	}

	/**
	 * GameRules(GLOBAL&)
	 * @brief Construct a GameRules instance from an INI file.
	 * @param cfg	- Ref to an INI instance.
	 */
	explicit GameRules(INI& cfg) :
		_walls_always_visible(cfg.get<bool>("world", "showAllWalls", conv::stob)),
		_override_known_tiles(cfg.get<bool>("world", "showAllTiles", conv::stob)),
		_dark_mode(cfg.get<bool>("world", "fogOfWar", conv::stob)),
		_cellSize({ cfg.get<long>("world", "sizeH", conv::stol), cfg.get<long>("world", "sizeV", conv::stol) }),
		_trap_dmg(cfg.get<int>("world", "trapDamage", conv::stoi)),
		_trap_percentage(cfg.get<bool>("world", "trapDamageIsPercentage", conv::stob)),
		_attack_cost_stamina(cfg.get<int>("actors", "attackCostStamina", conv::stoi)),
		_attack_block_chance(cfg.get<float>("actors", "attackBlockChance", conv::stof)),
		_attack_miss_chance_full(cfg.get<float>("actors", "attackMissChanceFull", conv::stof)),
		_attack_miss_chance_drained(cfg.get<float>("actors", "attackMissChanceDrained", conv::stof)),
		_player_godmode(cfg.get<bool>("player", "godmode", conv::stob)),
		_npc_move_chance(cfg.get<float>("actors", "npcMoveChance", conv::stof)),
		_npc_move_chance_aggro(cfg.get<float>("actors", "npcMoveChanceAggro", conv::stof)),
		_npc_vis_mod_aggro(cfg.get<int>("actors", "npcVisModAggro", conv::stoi)),
		_enemy_count(cfg.get<int>("enemy", "count", conv::stoi)),
		_enemy_aggro_distance(cfg.get<int>("enemy", "aggroDistance", conv::stoi)),
		_neutral_count(cfg.get<int>("neutral", "count", conv::stoi)),
		_regen_timer(cfg.get<int>("actors", "regen_time", conv::stoi)),
		_regen_health(cfg.get<int>("actors", "regen_health", conv::stoi)),
		_regen_stamina(cfg.get<int>("actors", "regen_stamina", conv::stoi)),
		_level_up_kills(cfg.get<int>("actors", "levelKillThreshold", conv::stoi)),
		_level_up_mult(cfg.get<int>("actors", "levelKillMult", conv::stoi)),
		_level_up_restore_percent(cfg.get<int>("actors", "levelRestorePercent", conv::stoi)),
		_enable_boss(cfg.get<bool>("enemy", "enable_boss", conv::stob)),
		_boss_spawns_after_final(cfg.get<bool>("enemy", "bossDelayedSpawn", conv::stob))
	{
		assert(!cfg.empty());
		// Set player stats
		if ( cfg.contains("player", "name") )		// Check name
			_player_template._name = cfg.get("player", "name");
		if ( cfg.contains("player", "health") )	// Check health
			_player_template._stats.setMaxHealth(cfg.get<int>("player", "health", conv::stoi));
		if ( cfg.contains("player", "stamina") )	// Check stamina
			_player_template._stats.setMaxStamina(cfg.get<int>("player", "stamina", conv::stoi));
		if ( cfg.contains("player", "damage") )	// Check damage
			_player_template._stats.setMaxDamage(cfg.get<int>("player", "damage", conv::stoi));

		buildTemplate(_player_template, cfg, "template_player");
		buildTemplate(_enemy_template.at(0), cfg, "template_enemy1");
		buildTemplate(_enemy_template.at(1), cfg, "template_enemy2");
		buildTemplate(_enemy_template.at(2), cfg, "template_enemy3");
		buildTemplate(_enemy_template.at(3), cfg, "template_enemy4");
		buildTemplate(_neutral_template.at(0), cfg, "template_neutral1");
		buildTemplate(_neutral_template.at(1), cfg, "template_neutral2");
		buildTemplate(_neutral_template.at(2), cfg, "template_neutral3");
	}

	// Default constructor
	GameRules() = default;
};