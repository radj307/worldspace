#pragma once
#include <string>
#include "Coord.h"
/**
 * WorldAttributes
 * Contains all settings used by world generation process.
 */
struct WorldAttributes {
	// default values:
	Coord _cellSize{ 12, 12 };
	bool _override_known_tiles{ false };
};

/**
 * GLOBAL <- WorldAttributes
 * Contains all global settings, and derived world attribute settings.
 */
struct GLOBAL : public WorldAttributes {
	// default values:
	std::string _import_filename{ "" };
	//Coord _resolution{ 800, 600 };
};