#pragma once
#include <atomic>
#include <string>
// Contains metadata about game outcome.
struct GameState {
	std::string _player_killed_by{};
	std::atomic<bool> // Game State Flags
		_final_challenge{ false },	// When true, all enemies (& neutrals if set in gamerules) attack the player.
		_allEnemiesDead{ false },	// When true, the player wins.
		_playerDead{ false },		// When true, the player loses.
		_game_is_over{ false };		// When true, the game is over.
};