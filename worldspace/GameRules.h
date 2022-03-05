/**
 * @file GameRules.h
 * @author radj307
 * @brief This file contains all configurable settings used in a game. These settings are members of the GameRules struct, an object that is required by the Gamespace to initialize.
 */
#pragma once
#include <cassert>
#include <chrono>
#include <strconv.hpp>
#include <TermAPI.hpp>
#include "actor.h"
#include "INI.hpp"

 /**
  * @struct GameRules
  * @brief Contains configurable game settings, their defaults, and configurable game methods, which are required for the game to operate.
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
	[[nodiscard]] bool CAN_LEVEL_UP(const unsigned level, const unsigned kills)
	{
		return kills > 0 && kills >= _level_up_kills * ((level == 0 ? 1 : level) * _level_up_mult);
	}

public:
	/// CELL / WORLD
	bool
		_walls_always_visible{ true },		///< @brief When true, wall tiles are always visible to the player.
		_override_known_tiles{ false },		///< @brief When true, the player can always see all tiles. Disables dark mode.
		_dark_mode{ false };				///< @brief When true, the player can only see the area around them.
	Coord _cellSize{ 30, 30 };				///< @brief If no filename is set, this is the size of the generated cell

	/// TRAPS
	int _trap_dmg{ 20 };					///< @brief the amount of health an actor loses when they step on a trap
	const bool _trap_percentage{ true };	///< @brief whether the _trap_dmg amount is static, or a percentage of max health

	/// ATTACKS
	int	_attack_cost_stamina{ 15 };			///< @brief The amount of stamina used when attacking. This is a static value.
	float
		_attack_block_chance{ 35.0f },		///< @brief Percentage chance of an attack being blocked when the target's stamina is high enough.
		_attack_miss_chance_full{ 11.0f },	///< @brief Percentage chance of an attack missing when the attacker's stamina is high enough.
		_attack_miss_chance_drained{ 35.0f }; ///< @brief Percentage chance of an attack missing when the attacker's stamina is not high enough.

	/// PLAYER
	bool _player_godmode{ false };			///< @brief When true, Player cannot be attacked
	ActorTemplate _player_template{ "Player", ActorStats(1, 120, 120, 45, 4), '$', color::green }; // Player template

	/// GENERIC NPC
	std::vector<FACTION>
		_enemy_hostile_to = { FACTION::PLAYER, /*FACTION::NEUTRAL*/ }, // Which factions are enemies hostile to by default.
		_neutral_hostile_to = { FACTION::NONE }; // Which factions are neutrals hostile to by default. This changes if NPC is attacked.
	float
		_npc_move_chance{ 60.0f },			///< @brief The chance an NPC will move when idle
		_npc_move_chance_aggro{ 90.0f };	///< @brief The chance an NPC will move when aggravated
	int _npc_vis_mod_aggro{ 1 };			///< @brief This value is added to an NPC's sight range when chasing target
	bool _level_stat_mult{ true };

	/// ENEMIES
	int
		_enemy_count{ 20 },					///< @brief how many enemies are present when the game starts
		_enemy_aggro_distance{ 2 };			///< @brief Determines from how far away enemies notice the player and become aggressive

	std::vector<ActorTemplate> _enemy_template{	///< @brief Default Enemy templates.
		{ "Bandit", ActorStats(1, 40, 100, 15, _enemy_aggro_distance + 1, _level_stat_mult), 'Y', Color::_f_yellow, _enemy_hostile_to, 30, 100.0f },	///< @brief Level 1 Enemy
		{ "Marauder", ActorStats(2, 40, 90, 13, _enemy_aggro_distance + 1, _level_stat_mult), 'T', Color::_f_red, _enemy_hostile_to, 20, 45.0f },		///< @brief Level 2 Enemy
		{ "Reaver", ActorStats(3, 60, 90, 30, _enemy_aggro_distance, _level_stat_mult), 'T', Color::_f_magenta, _enemy_hostile_to, 20, 20.0f },		///< @brief Level 3 Enemy
		{ "Reaper", ActorStats(4, 60, 100, 30, _enemy_aggro_distance, _level_stat_mult), 'M', Color::_f_magenta, _enemy_hostile_to, 30, 2.0f },		///< @brief Level 4 Enemy
	};

	/// BOSSES
	std::vector<ActorTemplate> _enemy_boss_template{
		{ "Grim Reaper", ActorStats(10, 25, 50, 40, _cellSize._x * _cellSize._y, _level_stat_mult), 'N', Color::_b_magenta, _enemy_hostile_to, 100, 0.0 },///< @brief Level 10 Enemy - (Boss: Grim Reaper)
		{ "Pit Boss", ActorStats(10, 25, 50, 40, _enemy_aggro_distance + 2, _level_stat_mult), 'N', Color::_b_magenta, _enemy_hostile_to, 100, 0.0 },		///< @brief Level 10 Enemy - (Boss: Pit Boss)
	};

	/// NEUTRALS
	int	_neutral_count{ 12 }; ///< @brief how many neutrals are present when the game starts
	std::vector<ActorTemplate> _neutral_template{ ///< @brief Neutral templates, aka default values for neutral types
		{ "Chicken", ActorStats(1, 30, 30, 5, 5, _level_stat_mult), '`', Color::_f_cyan, _neutral_hostile_to, 100, 100.0f },	///< Level 1 Neutral
		{ "Sheep", ActorStats(2, 30, 30, 5, 4, _level_stat_mult), '@', Color::_f_cyan, _neutral_hostile_to, 50, 45.0f },		///< Level 2 Neutral
		{ "Cow", ActorStats(3, 30, 30, 5, 4, _level_stat_mult), '%', Color::_f_blue, _neutral_hostile_to, 35, 20.0f },		///< Level 3 Neutral
	};

	/// PASSIVE EFFECTS
	std::chrono::seconds _regen_timer{ 2 };	///< @brief Amount of time between stat regen cycles
	int
		_regen_health{ 5 },					///< @brief Amount of health regenerated every second
		_regen_stamina{ 10 };				///< @brief Amount of stamina regenerated every second

	/// LEVELS
	unsigned
		_level_up_kills{ 2 },				///< @brief Kills required for an actor to level up. This value is arbitrary.
		_level_up_mult{ 2 },				///< @brief Multiplies the level_up_kills threshold for every level
		_level_up_restore_percent{ 50 };	///< @brief How much health/stamina is regenerated when the player levels up ( 0 - 100 )

	/**
	 * canLevelUp(ActorBase*)
	 * @brief Checks if a given actor can level up, based on their current stats.
	 * @param actor	 - Pointer to an actor
	 * @returns bool - ( true = can level up ) ( false = cannot level up )
	 */
	bool canLevelUp(ActorBase* actor)
	{
		return static_cast<unsigned>(actor->getKills()) > _level_up_kills * actor->getLevel();
		/*	if (_level_up_kills == 2)
				return actor != nullptr && static_cast<float>(actor->getLevel() * _level_up_mult) / 1.5f > _level_up_kills;
			else
				throw std::exception("CORRUPTED_LEVELING_DATA");
	*///		return actor != nullptr && CAN_LEVEL_UP(actor->getLevel(), actor->getKills());
	}

	unsigned short
		_level_up_flare_time{ 6 };			///< @brief How many frames to flare when the player levels up. Must be a multiple of 2
	unsigned int
		_challenge_final_trigger_percent{ 25 }; ///< @brief Percentage of remaining enemies to trigger finale. (0 to disable.)
	bool
		_challenge_neutral_is_hostile{ false }, ///< @brief When true, all Neutral NPCs become hostile to the player during the final challenge.
		_enable_boss{ true }, ///< @brief When true, a boss will spawn before the end of the game
		_boss_spawns_after_final{ true }; ///< @brief When true, the boss will only spawn after the final challenge.

	///< @brief Possible messages to show for "killed by:" when player died from a trap
	std::vector<std::string> _killed_by_trap{
		"trap",
		"a hole in the floor",
		"shattered legs",
		"falling into the abyss",
	};

	/**
	 * buildTemplate(INI&, string&)
	 * @brief Builds an ActorTemplate from the specified INI section.
	 * @param target	- Ref of the target template instance.
	 * @param cfg		- Ref to the INI instance
	 * @param section	- INI section name containing variables
	 */
	static void setINITemplate(ActorTemplate& target, file::INI& cfg, const std::string& section)
	{
		if (cfg.contains(section)) {
			target = ActorTemplate{
				cfg.get(section, "name").value_or(target._name),
				ActorStats(cfg.get<int>(section, "level", str::stoi).value_or(target._stats.getLevel()),
				cfg.get<int>(section, "health", str::stoi).value_or(target._stats.getHealth()),
				cfg.get<int>(section, "stamina", str::stoi).value_or(target._stats.getStamina()),
				cfg.get<int>(section, "damage", str::stoi).value_or(target._stats.getMaxDamage()),
				cfg.get<int>(section, "visRange", str::stoi).value_or(target._stats.getVis())),
				cfg.get<char>(section, "char", str::stoc).value_or(target._char),
				cfg.get<unsigned short>(section, "color", Color::strToColor).value_or(target._color),
				strToFactions(cfg.get(section, "hostileTo").value_or("")).value_or(target._hostile_to),
				cfg.get<int>(section, "maxAggro", str::stoi).value_or(target._max_aggression),
				cfg.get<float>(section, "spawnChance", str::stof).value_or(target._chance)
			};
		} // else do nothing
	}

	/**
	 * GameRules(GLOBAL&)
	 * @brief Construct a GameRules instance from an INI file.
	 * @param cfg	- Ref to an INI instance.
	 */
	explicit GameRules(file::INI& cfg) :
		_walls_always_visible(cfg.get<bool>("world", "showAllWalls", str::stob).value_or(_walls_always_visible)),
		_override_known_tiles(cfg.get<bool>("world", "showAllTiles", str::stob).value_or(_override_known_tiles)),
		_dark_mode(cfg.get<bool>("world", "fogOfWar", str::stob).value_or(_dark_mode)),
		_cellSize(cfg.get<long>("world", "sizeH", str::stol).value_or(_cellSize._x), cfg.get<long>("world", "sizeV", str::stol).value_or(_cellSize._y)),
		_trap_dmg(cfg.get<int>("world", "trapDamage", str::stoi).value_or(_trap_dmg)),
		_trap_percentage(cfg.get<bool>("world", "trapDamageIsPercentage", str::stob).value_or(_trap_percentage)),
		_attack_cost_stamina(cfg.get<int>("actors", "attackCostStamina", str::stoi).value_or(_attack_cost_stamina)),
		_attack_block_chance(cfg.get<float>("actors", "attackBlockChance", str::stof).value_or(_attack_block_chance)),
		_attack_miss_chance_full(cfg.get<float>("actors", "attackMissChanceFull", str::stof).value_or(_attack_miss_chance_full)),
		_attack_miss_chance_drained(cfg.get<float>("actors", "attackMissChanceDrained", str::stof).value_or(_attack_miss_chance_drained)),
		_player_godmode(cfg.get<bool>("player", "godmode", str::stob).value_or(_player_godmode)),
		_npc_move_chance(cfg.get<float>("actors", "npcMoveChance", str::stof).value_or(_npc_move_chance)),
		_npc_move_chance_aggro(cfg.get<float>("actors", "npcMoveChanceAggro", str::stof).value_or(_npc_move_chance_aggro)),
		_npc_vis_mod_aggro(cfg.get<int>("actors", "npcVisModAggro", str::stoi).value_or(_npc_vis_mod_aggro)),
		_level_stat_mult(cfg.get<bool>("actors", "multStatsByLevel", str::stob)),
		_enemy_count(cfg.get<int>("enemy", "count", str::stoi).value_or(_enemy_count)),
		_enemy_aggro_distance(cfg.get<int>("enemy", "aggroDistance", str::stoi).value_or(_enemy_aggro_distance)),
		_neutral_count(cfg.get<int>("neutral", "count", str::stoi).value_or(_neutral_count)),
		_regen_timer(cfg.get<int>("actors", "regen_time", str::stoi).value_or(2)),
		_regen_health(cfg.get<int>("actors", "regen_health", str::stoi).value_or(_regen_health)),
		_regen_stamina(cfg.get<int>("actors", "regen_stamina", str::stoi).value_or(_regen_stamina)),
		//		_level_up_kills					(cfg.get<int>	("player", "levelKillThreshold", str::stoi).value_or(_level_up_kills)),
		//		_level_up_mult					(cfg.get<int>	("player", "levelKillMult", str::stoi).value_or(_level_up_mult)),
		_level_up_restore_percent(cfg.get<int>("actors", "levelRestorePercent", str::stoi).value_or(_level_up_restore_percent)),
		_enable_boss(cfg.get<bool>("enemy", "enable_boss", str::stob).value_or(_enable_boss)),
		_boss_spawns_after_final(cfg.get<bool>("enemy", "bossDelayedSpawn", str::stob).value_or(_boss_spawns_after_final))
	{
		assert(!cfg.empty());
		///< @brief Set player stats
		if (cfg.contains("player", "name"))		///< @brief Check name
			_player_template._name = cfg.get("player", "name").value_or(_player_template._name);
		if (cfg.contains("player", "health"))	///< @brief Check health
			_player_template._stats.setMaxHealth(cfg.get<int>("player", "health", str::stoi).value_or(_player_template._stats.getMaxHealth()));
		if (cfg.contains("player", "stamina"))	///< @brief Check stamina
			_player_template._stats.setMaxStamina(cfg.get<int>("player", "stamina", str::stoi).value_or(_player_template._stats.getMaxStamina()));
		if (cfg.contains("player", "damage"))	///< @brief Check damage
			_player_template._stats.setMaxDamage(cfg.get<int>("player", "damage", str::stoi).value_or(_player_template._stats.getMaxDamage()));

		setINITemplate(_player_template, cfg, "template_player");
		for (size_t i{ 0 }; i < _enemy_template.size(); ++i)
			setINITemplate(_enemy_template.at(i), cfg, std::string("template_enemy") + std::to_string(i + 1));
		for (size_t i{ 0 }; i < _enemy_boss_template.size(); ++i)
			setINITemplate(_enemy_boss_template.at(i), cfg, std::string("template_boss") + std::to_string(i + 1));
		for (size_t i{ 0 }; i < _neutral_template.size(); ++i)
			setINITemplate(_neutral_template.at(i), cfg, std::string("template_neutral") + std::to_string(i + 1));
	}

	GameRules() = default; ///< @brief Default Constructor.
};
