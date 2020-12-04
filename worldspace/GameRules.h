#pragma once
#include "actor.h"
#include "settings.h"

/**
 * struct GameRules
 * Gamespace requires an instance of GameRules to construct, these are generic settings that may be used by a game to allow user-customization.
 */
struct GameRules {
	// CELL / WORLD
	bool
		_walls_always_visible{ true },		// When true, wall tiles are always visible to the player.
		_override_known_tiles{ false };		// When true, the player can always see all tiles.
	std::string _world_import_file{};		// The game will attempt to load cell from this file. If blank, generates a new cell.
	Coord _cellSize{ 30, 30 };				// If no filename is set, this is the size of the generated cell

	// TRAPS
	int _trap_dmg{ 20 };					// the amount of health an actor loses when they step on a trap
	const bool _trap_percentage{ true };	// whether the _trap_dmg amount is static, or a percentage of max health

	// ATTACKS
	int _attack_cost_stamina{ 15 };			// The amount of stamina used when attacking. This is a static value.

	// PLAYER
	bool _player_godmode{ false };			// When true, Player cannot be attacked
	ActorTemplate _player_template{ "Player", ActorStats(1, 120, 120, 45, 4), '$', WinAPI::color::green }; // Player template

	// NPC RELATIONSHIPS
	std::vector<FACTION>
		_enemy_hostile_to =	{ FACTION::PLAYER, FACTION::NEUTRAL }, // Which factions are enemies hostile to by default.
		_neutral_hostile_to = { FACTION::NONE }; // Which factions are neutrals hostile to by default. This changes if NPC is attacked.
	
	// ENEMY-SPECIFIC
	int
		_enemy_count{ 20 },					// how many enemies are present when the game starts
		_enemy_move_chance{ 4 },			// 1 in (this) chance of a hostile moving each cycle (range is 0-this)
		_enemy_aggro_distance{ 4 };			// Determines from how far away enemies notice the player and become aggressive
	std::vector<ActorTemplate> _enemy_template{		// Enemy templates, aka default values for enemy types
		{ "Bandit",	 ActorStats(1, 40, 55, 15, _enemy_aggro_distance), 'Y', WinAPI::color::yellow, _enemy_hostile_to, 50, 30 },	// Level 1 enemy
		{ "Marauder",ActorStats(2, 40, 45, 20, _enemy_aggro_distance), 'T', WinAPI::color::red, _enemy_hostile_to, 35, 25 },	// Level 2 enemy
		{ "Reaver",	 ActorStats(3, 60, 60, 30, _enemy_aggro_distance + 1), 'T', WinAPI::color::magenta, _enemy_hostile_to, 15, 15 },// Level 3 enemy
	};

	// NEUTRALS
	int	_neutral_count{ 20 };				// how many neutrals are present when the game starts
	std::vector<ActorTemplate> _neutral_template{	// Neutral templates, aka default values for neutral types
		{ "Chicken",ActorStats(1, 20, 25, 5, 3), '`', WinAPI::color::cyan, _neutral_hostile_to, 50, 11 },
		{ "Sheep",	ActorStats(2, 25, 25, 10, 4), '@', WinAPI::color::cyan, _neutral_hostile_to, 35, 11 },
		{ "Cow",	ActorStats(3, 40, 25, 15, 5), '%', WinAPI::color::blue, _neutral_hostile_to, 15, 11 },
	};

	// PASSIVE EFFECTS
	std::chrono::seconds _regen_timer{ 2 };
	int
		_regen_health{ 5 },					// Amount of health regenerated every second
		_regen_stamina{ 10 };				// Amount of stamina regenerated every second

	// LEVELS
	int
		_level_up_kills{ 2 },				// Kills required for an actor to level up. This value is arbitrary.
		_level_up_mult{ 2 },				// Multiplies the level_up_kills threshold for every level
		_level_up_flare_time{ 6 },			// How many frames to flare when the player levels up. Must be a multiple of 2
		_level_up_restore_percent{ 50 };	// How much health/stamina is regenerated when the player levels up ( 0 - 100 )
	
	// The level up algorithm
	[[nodiscard]] constexpr bool CAN_LEVEL_UP(const int level, const int kills) const { return kills >= _level_up_kills * (level * _level_up_mult); }
	// The level-up flare pattern
	[[nodiscard]] static constexpr bool flare_pattern(const Coord pos) { return (pos._x - pos._y % 2) % 2 == 0; }
	
	// Default constructor
	GameRules() = default;

	/**
	 * canLevelUp(ActorBase*)
	 * Returns true if a given actor can level up.
	 *
	 * @param actor	 - Pointer to an actor
	 * @returns bool - ( true = can level up ) ( false = cannot level up )
	 */
	bool canLevelUp(ActorBase* actor) const
	{
		if ( actor != nullptr && CAN_LEVEL_UP(actor->getLevel(), actor->getKills()) )
			return true;
		return false;
	}
	
	/**
	 * GameRules(GLOBAL&)
	 * Construct a GameRules instance from a GLOBAL settings instance.
	 *
	 * @param settings	- Ref to a GLOBAL instance containing parsed commandline settings
	 */
	explicit GameRules(const GLOBAL& settings) : _override_known_tiles(settings._override_known_tiles), _world_import_file(settings._import_filename), _cellSize(settings._cellSize), _player_godmode(settings._player_godmode), _regen_timer(settings._regen_timer)
	{
		// Set player stats
		if ( settings._player_health != NOT_SET )	// Check health
			_player_template._stats.setHealth(settings._player_health);
		if ( settings._player_stamina != NOT_SET )	// Check stamina
			_player_template._stats.setStamina(settings._player_stamina);
		if ( settings._player_damage != NOT_SET )	// Check damage
			_player_template._stats.setMaxDamage(settings._player_damage);
	}
};