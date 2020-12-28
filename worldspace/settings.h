#pragma once
/**
 * struct CONTROLS
 * @brief Used to define game key bindings
 */
struct CONTROLS {
	// PLAYER CONTROLS
	char _KEY_UP, _KEY_DOWN, _KEY_LEFT, _KEY_RIGHT, _KEY_PAUSE, _KEY_QUIT;

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
	explicit CONTROLS(const char up = 'w', const char down = 's', const char left = 'a', const char right = 'd', const char pause = 'p', const char quit = 'q') : _KEY_UP(up), _KEY_DOWN(down), _KEY_LEFT(left), _KEY_RIGHT(right), _KEY_PAUSE(pause), _KEY_QUIT(quit) {}

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
// Default controls instance
inline CONTROLS _CTRL;
// Pointer used to define the current control set.
inline CONTROLS* __controlset{ nullptr };