/**
 * @file controls.h
 * @brief This file contains the control scheme and the _current_control_set pointer.
 *\n
 * _current_control_set*
 * Set this to the control set, it is checked throughout the program whenever a key press is checked.
 */
#pragma once
#include <INI.hpp>

#include <map>
#include <optional>
#include <string>
#include <utility>

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

	std::vector<char> getKeyBindsFor(const Control& ctrl) const
	{
		const auto& str{ bindings.at(ctrl) };
		std::vector<char> vec;
		vec.reserve(str.size());
		for (const auto& ch : str)
			vec.emplace_back(ch);
		vec.shrink_to_fit();
		return vec;
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
