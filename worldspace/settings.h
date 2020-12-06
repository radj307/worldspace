#pragma once
#include <chrono>
#include <string>
#include "Coord.h"
constexpr int NOT_SET = -1;
/**
 * GLOBAL <- WorldAttributes
 * Contains all global settings, and derived world attribute settings.
 */
struct GLOBAL {
	// WORLD ATTRIBUTES & PARAMETERS
	Coord _cellSize{ 30, 30 };				// Defines the size of a cell
	bool _override_known_tiles{ false };	// When true, the player can always see all tiles.
	std::string _import_filename{};		// When not blank, the cell will be loaded from the filename defined here.

	// GAME PARAMETERS
	std::chrono::seconds _regen_timer{ 2 };
	std::string _player_name{};
	int _player_health{ NOT_SET }, _player_stamina{ NOT_SET }, _player_damage{ NOT_SET };
	bool _player_godmode{ false };
};