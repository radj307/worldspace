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

	// GENERIC NPC
	std::vector<FACTION>
		_enemy_hostile_to =	{ FACTION::PLAYER, /*FACTION::NEUTRAL*/ }, // Which factions are enemies hostile to by default.
		_neutral_hostile_to = { FACTION::NONE }; // Which factions are neutrals hostile to by default. This changes if NPC is attacked.
	int _npc_move_chance{ 6 };
	
	// ENEMIES
	int
		_enemy_count{ 20 },					// how many enemies are present when the game starts
		_enemy_aggro_distance{ 3 };			// Determines from how far away enemies notice the player and become aggressive
	// ActorStats((Level), (Health), (Stamina), (Damage), (Visibility)), (Character), (Color), (Hostile factions), (Max aggression val in move cycles), (Chance to spawn))
	// Chance to spawn is calculated starting at the end of the list, so the first template should have a chance of 100
	std::vector<ActorTemplate> _enemy_template{		// Enemy templates, aka default values for enemy types.
		{ "Bandit",	 ActorStats(1, 40, 100, 15, _enemy_aggro_distance), 'Y', WinAPI::color::yellow, _enemy_hostile_to, 30, 100 },	// Level 1 enemy
		{ "Marauder",ActorStats(2, 40, 90, 13, _enemy_aggro_distance), 'T', WinAPI::color::red, _enemy_hostile_to, 20, 40 },	// Level 2 enemy
		{ "Reaver",	 ActorStats(3, 60, 90, 30, _enemy_aggro_distance), 'T', WinAPI::color::magenta, _enemy_hostile_to, 16, 15 },// Level 3 enemy
		{ "Reaper",	 ActorStats(4, 60, 100, 30, _enemy_aggro_distance), 'M', WinAPI::color::magenta, _enemy_hostile_to, 14, 1 },// Level 4 enemy
	};

	// NEUTRALS
	int	_neutral_count{ 12 };				// how many neutrals are present when the game starts
	std::vector<ActorTemplate> _neutral_template{	// Neutral templates, aka default values for neutral types
		{ "Chicken",ActorStats(1, 30, 30, 5, 5), '`', WinAPI::color::cyan, _neutral_hostile_to, 100, 100 },
		{ "Sheep",	ActorStats(2, 30, 30, 5, 4), '@', WinAPI::color::cyan, _neutral_hostile_to, 50, 45 },
		{ "Cow",	ActorStats(3, 30, 30, 5, 4), '%', WinAPI::color::blue, _neutral_hostile_to, 35, 20 },
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
		_level_up_restore_percent{ 50 };	// How much health/stamina is regenerated when the player levels up ( 0 - 100 )
	unsigned short
		_level_up_flare_time{ 6 };			// How many frames to flare when the player levels up. Must be a multiple of 2
	
	// The level up algorithm
	[[nodiscard]] constexpr bool CAN_LEVEL_UP(const int level, const int kills) const { return kills >= _level_up_kills * (level * _level_up_mult); }

	// CHALLENGES
	unsigned int // Percentage of remaining enemies to trigger finale. (0 to disable.)
		_challenge_final_trigger_percent{ 25 }; 
	bool		 // if true, neutral NPCs will also attack the player during the finale
		_challenge_neutral_is_hostile{ false }; 
	
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
		if ( !settings._player_name.empty() )		// Check name
			_player_template._name = settings._player_name;
		if ( settings._player_health != NOT_SET )	// Check health
			_player_template._stats.setMaxHealth(settings._player_health);
		if ( settings._player_stamina != NOT_SET )	// Check stamina
			_player_template._stats.setMaxStamina(settings._player_stamina);
		if ( settings._player_damage != NOT_SET )	// Check damage
			_player_template._stats.setMaxDamage(settings._player_damage);
	}
};