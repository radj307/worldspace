/**
 * @file controls.h
 * @brief This file contains the control scheme and the _current_control_set pointer.
 *\n
 * _current_control_set*
 * Set this to the control set, it is checked throughout the program whenever a key press is checked.
 */
#pragma once
#include <utility>

enum class Dir : int {
	UP = 0,
	RIGHT = 1,
	DOWN = 2,
	LEFT = 3,
	NONE = 4,
};

inline Dir invert(const Dir& dir)
{
	switch (dir) {
	case Dir::NONE:
		return dir;
	default: {
		auto v{ static_cast<int>(dir) - 2 };
		while (v < 0)
			v = static_cast<int>(Dir::LEFT) - v;
		return static_cast<Dir>(v);
	}
	}
}

enum class Control : unsigned char {
	NULL_CONTROL = 255,
	UP = 0,
	RIGHT = 1,
	DOWN = 2,
	LEFT = 3,
	PAUSE = 10,
	RESTART = 11,
	QUIT = 12,
};

inline Dir toDir(const Control& ctrl) noexcept(false)
{
	if (const auto& v{ static_cast<unsigned char>(ctrl) }; v < 0 || v > 3)
		throw make_exception("Invalid control => direction conversion: Control value \'", v, "\' isn't a valid direction!");
	return static_cast<Dir>(ctrl);
}

inline Control fromDir(const Dir& dir) noexcept
{
	return static_cast<Control>(dir);
}

struct Controls {
	std::map<Control, std::string> bindings{
		{ Control::UP, "w" },
		{ Control::DOWN, "s" },
		{ Control::LEFT, "a" },
		{ Control::RIGHT, "d" },
		{ Control::PAUSE, "p" },
		{ Control::RESTART, "r" },
		{ Control::QUIT, "q" },
	};

	Controls() = default;
	Controls(const file::INI& ini) { importINI(ini); }

	Control fromKey(const char& keycode) const
	{
		for (const auto& [ctrl, keys] : bindings)
			if (keys.find_first_of(keycode) < keys.size())
				return ctrl;
		return Control::NULL_CONTROL;
	}

	Control fromDirection(const int& val) const noexcept
	{
		const Control ctrl{ static_cast<Control>(val) };
		switch (ctrl) {
		case Control::UP: [[fallthrough]];
		case Control::DOWN: [[fallthrough]];
		case Control::LEFT: [[fallthrough]];
		case Control::RIGHT:
			return ctrl;
		default:
			return Control::NULL_CONTROL;
		}
	}

	bool importINI(const file::INI& ini)
	{
		if (ini.check_header("controls")) {
			const auto& loadKey{ [&ini, this](const std::string& keyName, const Control& ctrl) {
				const auto val{ ini.getvs("controls", keyName) };
				if (val.has_value())
					bindings.insert_or_assign(ctrl, val.value());
			} };
			loadKey("key_up", Control::UP);
			loadKey("key_down", Control::DOWN);
			loadKey("key_left", Control::LEFT);
			loadKey("key_right", Control::RIGHT);
			loadKey("key_pause", Control::PAUSE);
			loadKey("key_restart", Control::RESTART);
			loadKey("key_quit", Control::QUIT);
			return true;
		}
		return false;
	}
	file::INI& exportINI(file::INI& ini) const
	{
		ini.set("controls", "key_up", bindings.at(Control::UP));
		ini.set("controls", "key_down", bindings.at(Control::DOWN));
		ini.set("controls", "key_left", bindings.at(Control::LEFT));
		ini.set("controls", "key_right", bindings.at(Control::RIGHT));
		ini.set("controls", "key_pause", bindings.at(Control::PAUSE));
		ini.set("controls", "key_restart", bindings.at(Control::RESTART));
		ini.set("controls", "key_quit", bindings.at(Control::QUIT));
		return ini;
	}
};

#ifdef USE_LEGACY_CONTROLS
/**
 * struct CONTROLS
 * @brief Defines all controls used by the game. Only affects the player.
 */
struct CONTROLS {
	// PLAYER CONTROLS
	const char
		_KEY_UP,      ///< Key to move up 1 tile
		_KEY_DOWN,    ///< Key to move down 1 tile
		_KEY_LEFT,    ///< Key to move left 1 tile
		_KEY_RIGHT,   ///< Key to move right 1 tile
		_KEY_PAUSE,   ///< Key to pause the game
		_KEY_QUIT,    ///< Key to quit the game
		_KEY_RESTART; ///< Key to restart the game (only used in the prompt_restart() function)

	/**
	 * CONTROLS(char, char, char, char, char, char)
	 * @brief Constructor that takes key vals as parameters.
	 *
	 * @param up		- Key to move up
	 * @param down		- Key to move down
	 * @param left		- Key to move left
	 * @param right		- Key to move right
	 * @param pause		- Key to pause/unpause
	 * @param quit		- Key to quit
	 * @param restart	- Key used to
	 */
	explicit CONTROLS(const char up = 'w', const char down = 's', const char left = 'a', const char right = 'd', const char pause = 'p', const char quit = 'q', const char restart = 'r') : _KEY_UP(up), _KEY_DOWN(down), _KEY_LEFT(left), _KEY_RIGHT(right), _KEY_PAUSE(pause), _KEY_QUIT(quit), _KEY_RESTART(restart) {}

	/**
	 * intToDir(int)
	 * @brief Converts an integer to a direction char
	 * @param i		 - Input integer between 0 and 3
	 * @returns char
	 * @return 'w'	- Up (0)
	 * @return 'd'	- Right (1)
	 * @return 's'	- Down (2)
	 * @return 'a'	- Left (3)
	 * @return ' '	- Invalid parameter
	 */
	[[nodiscard]] char intToDir(int i) const
	{
		if (i < 0) i = -i % 4; else if (i > 3) i %= 4;
		switch (i) {
		case 0:return _KEY_UP;
		case 1:return _KEY_RIGHT;
		case 2:return _KEY_DOWN;
		case 3:return _KEY_LEFT;
		case -404:
		default:return ';';
		}
	}

	/**
	 * intToDir(int)
	 * @brief Converts an integer to a direction char
	 * @param c		- Input integer between 0 and 3
	 * @returns int
	 * @return 0	- Up ('w')
	 * @return 1	- Right ('d')
	 * @return 2	- Down ('s')
	 * @return 3	- Left ('a')
	 * @return -404	- Invalid parameter
	 */
	[[nodiscard]] int dirToInt(const char c) const
	{
		if (c == ';')
			return -404;
		if (c == _KEY_UP)
			return 0;
		if (c == _KEY_RIGHT)
			return 1;
		if (c == _KEY_DOWN)
			return 2;
		if (c == _KEY_LEFT)
			return 3;
		return -404;
	}

	[[nodiscard]] char reverse(const char c) const { return intToDir(dirToInt(c) - 2); }
	[[nodiscard]] int reverse(const int i) const { return dirToInt(intToDir(i - 2)); }
};
// Default controls instance
inline CONTROLS _CTRL;
// Pointer to default controls that can be overridden to modify the control set
inline CONTROLS* _current_control_set{ &_CTRL };
#endif
