#pragma once
#include <chrono>
#include <string>
#include "Coord.h"
#include "INI_Include.h"

static const int NOT_SET = -1; // Unset value used for checking if variables should be used. (Equal to -1)

/**
 * struct GLOBAL
 * Contains all global settings, and derived world attribute settings.
 */
struct GLOBAL {
	// WORLD ATTRIBUTES & PARAMETERS
	Coord _cellSize{ 30, 30 };				// Defines the size of a cell
	bool _override_known_tiles{ false };	// When true, the player can always see all tiles.
	std::string _import_filename{};		// When not blank, the cell will be loaded from the filename defined here.

	// GAME PARAMETERS
	std::chrono::seconds _regen_timer{ 2 };
	
	// PLAYER ATTRIBUTES
	std::string _player_name{};
	int _player_health{ NOT_SET }, _player_stamina{ NOT_SET }, _player_damage{ NOT_SET };
	bool _player_godmode{ false };
};

/**
 * struct CONTROLS
 */
struct CONTROLS {
	// PLAYER CONTROLS
	const char _KEY_UP, _KEY_DOWN, _KEY_LEFT, _KEY_RIGHT, _KEY_PAUSE, _KEY_QUIT;

	/**
	 * CONTROLS()
	 * @brief Default constructor that loads key vals from INI
	 */
	CONTROLS() : _KEY_UP(static_cast<char>(cfg.iGet("Key_Up"))), _KEY_DOWN(static_cast<char>(cfg.iGet("Key_Down"))), _KEY_LEFT(static_cast<char>(cfg.iGet("Key_Left"))), _KEY_RIGHT(static_cast<char>(cfg.iGet("Key_Right"))), _KEY_PAUSE(static_cast<char>(cfg.iGet("Key_Pause"))), _KEY_QUIT(static_cast<char>(cfg.iGet("Key_Quit"))) {}
	
	/**
	 * CONTROLS(char, char, char, char, char, char)
	 * @brief Constructor that takes key vals as parameters.
	 *
	 * @param up	- Key to move up
	 * @param down	- Key to move down
	 * @param left	- Key to move left
	 * @param right	- Key to move right
	 * @param pause	- Key to pause/unpause
	 * @param quit	- Key to quit
	 */
	explicit CONTROLS(const char up, const char down, const char left, const char right, const char pause, const char quit) : _KEY_UP(up), _KEY_DOWN(down), _KEY_LEFT(left), _KEY_RIGHT(right), _KEY_PAUSE(pause), _KEY_QUIT(quit) {}

	/**
	 * intToDir(int)
	 * @brief Converts an integer to a direction char
	 * @param i		 - Input integer between 0 and 3
	 * @returns char - ( 'w' == 0 ) ( 'd' == 1 ) ( 's' == 2 ) ( 'a' == 3 ) ( ' ' == invalid parameter )
	 */
	[[nodiscard]] char intToDir(const int i) const
	{
		switch ( i ) {
		case 0: return _KEY_UP;
		case 1: return _KEY_RIGHT;
		case 2: return _KEY_DOWN;
		case 3: return _KEY_LEFT;
		default:return ' ';
		}
	}

	/**
	 * intToDir(int)
	 * @brief Converts an integer to a direction char
	 * @param c		 - Input integer between 0 and 3
	 * @returns int - ( 0 == 'w' ) ( 1 == 'd' ) ( 2 == 's' ) ( 3 == 'a' ) ( -1 == invalid parameter )
	 */
	[[nodiscard]] int dirToInt(const char c) const
	{
		if ( c == _KEY_UP )
			return 0;
		if ( c == _KEY_RIGHT )
			return 1;
		if ( c == _KEY_DOWN )
			return 2;
		if ( c == _KEY_LEFT )
			return 3;
		return -1;
	}
};

/**
 * getControlsFromINI()
 * @brief Construct a CONTROLS instance
 * @returns CONTROLS
 */
inline CONTROLS getControlsFromINI()
{
	// convert to keys first because constructor refuses to accept static casts as param
	const auto _KEY_UP{ static_cast<char>(cfg.iGet("Key_Up")) }, _KEY_DOWN{ static_cast<char>(cfg.iGet("Key_Down")) }, _KEY_LEFT{ static_cast<char>(cfg.iGet("Key_Left")) }, _KEY_RIGHT{ static_cast<char>(cfg.iGet("Key_Right")) }, _KEY_PAUSE{ static_cast<char>(cfg.iGet("Key_Pause")) }, _KEY_QUIT{ static_cast<char>(cfg.iGet("Key_Quit")) };
	return CONTROLS(_KEY_UP, _KEY_DOWN, _KEY_LEFT, _KEY_RIGHT, _KEY_PAUSE, _KEY_QUIT);
}
// Default controls instance
inline CONTROLS _CTRL{ getControlsFromINI() };
// Pointer to default controls that can be overridden to modify the control set
inline CONTROLS* __controlset{ &_CTRL };