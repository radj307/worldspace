#pragma once
#include "actor.h"

/**
 * struct GameRules
 * Gamespace requires an instance of GameRules to construct, these are generic settings that may be used by a game to allow user-customization.
 */
struct GameRules {
	// CELL / WORLD
	bool _walls_always_visible{ true };		// When true, wall tiles are always visible to the player.

	// TRAPS
	int _trap_dmg{ 20 };					// the amount of health an actor loses when they step on a trap
	const bool _trap_percentage{ true };			// whether the _trap_dmg amount is static, or a percentage of max health

	// ATTACKS
	int _attack_cost_stamina{ 15 };			// The amount of stamina used when attacking.

	// PLAYER
	ActorTemplate _player_template{ "Player", ActorStats(1, 100, 100, 45, 4), '$', WinAPI::color::green };

	// NPC RELATIONSHIPS
	std::vector<FACTION>				// Which factions are enemies hostile to by default
	_enemy_hostile_to =	
	{
		FACTION::PLAYER,
		FACTION::NEUTRAL,
	},
	_neutral_hostile_to =				// Which factions are neutrals hostile to by default
	{
		FACTION::NONE,
	};
	
	// ENEMIES
	std::vector<ActorTemplate> _enemy_template{
		{ "Bandit",	 ActorStats(1, 40, 45, 10, 3), 'Y', WinAPI::color::yellow, _enemy_hostile_to, 50 },
		{ "Marauder",ActorStats(2, 50, 45, 20, 4), 'T', WinAPI::color::red, _enemy_hostile_to, 35 },
		{ "Reaver",	 ActorStats(3, 60, 60, 30, 5), 'T', WinAPI::color::magenta, _enemy_hostile_to, 15 },
	};
	int
		_enemy_count{ 20 },				// how many enemies are present when the game starts
		_enemy_move_chance{ 4 },		// 1 in (this) chance of a hostile moving each cycle (range is 0-this)
		_enemy_aggro_distance{ 4 },		// Determines from how far away enemies notice the player and become aggressive
		_enemy_aggro_duration{ 20 };	// Determines how long enemies will remain aggressive

	// NEUTRALS
	std::vector<ActorTemplate> _neutral_template{
		{ "Chicken",ActorStats(1, 20, 25, 5, 3), '`', WinAPI::color::cyan, _neutral_hostile_to, 50 },
		{ "Sheep",	ActorStats(2, 25, 25, 10, 4), '@', WinAPI::color::cyan, _neutral_hostile_to, 35 },
		{ "Cow",	ActorStats(3, 40, 25, 15, 5), '%', WinAPI::color::blue, _neutral_hostile_to, 15 },
	};
	int	_neutral_count{ 10 };

	// LEVELS
	int _level_up_kills{ 2 };			// Kills required for an actor to level up
	float _level_up_mult{ 1.5f };		// Multiplier for the level up kills threshold, this is applied after every level

	// PASSIVE EFFECTS
	int
		_regen_health{ 5 },				// Amount of health regenerated every second
		_regen_stamina{ 10 };			// Amount of stamina regenerated every second

	bool canLevelUp(ActorBase* a) const
	{
		if ( (a != nullptr) && !(a->isDead()) && (a->getKills() >= (static_cast<int>(std::round(static_cast<float>(a->getLevel()) * _level_up_mult)) * _level_up_kills)) )
			return true;
		return false;
	}
};