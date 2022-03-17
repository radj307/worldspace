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

#define MULTICHAR_SEQUENCE '\340'
#define ARROW_UP '\110'
#define ARROW_RIGHT '\115'
#define ARROW_DOWN '\120'
#define ARROW_LEFT '\113'

enum class Control : unsigned char {
	NULL_CONTROL = 255,
	UP = 0,
	RIGHT = 1,
	DOWN = 2,
	LEFT = 3,
	PAUSE = 4,
	RESTART = 5,
	QUIT = 6,
	SEQUENCE = 7,
	FIRE_UP = 10,
	FIRE_RIGHT = 11,
	FIRE_DOWN = 12,
	FIRE_LEFT = 13,
};

struct KeyBind {
	std::string keyCodes;

	KeyBind(std::string&& keys) : keyCodes{ std::move(keys) } {}
	KeyBind(char&& key) : keyCodes{ str::ctos(std::move(key)) } {}

	operator const std::string& () const { return keyCodes; }

	size_t size() const { return keyCodes.size(); }
	auto begin() const { return keyCodes.begin(); }
	auto end() const { return keyCodes.end(); }

	bool operator()(const char& key) const
	{
		return keyCodes.find(key) != std::string::npos;
	}
};

struct Controls {
	std::map<Control, KeyBind> bindings{
		std::make_pair(Control::UP, KeyBind("w")),
		std::make_pair(Control::DOWN, KeyBind("s")),
		std::make_pair(Control::LEFT, KeyBind("a")),
		std::make_pair(Control::RIGHT, KeyBind("d")),
		std::make_pair(Control::PAUSE, KeyBind("p")),
		std::make_pair(Control::RESTART, KeyBind("r")),
		std::make_pair(Control::QUIT, KeyBind("q")),
		std::make_pair(Control::FIRE_UP, KeyBind(ARROW_UP)),
		std::make_pair(Control::FIRE_DOWN, KeyBind(ARROW_DOWN)),
		std::make_pair(Control::FIRE_LEFT, KeyBind(ARROW_LEFT)),
		std::make_pair(Control::FIRE_RIGHT, KeyBind(ARROW_RIGHT)),
	};

	Controls() = default;
	Controls(const file::INI& ini) { importINI(ini); }

	Control fromKey(const char& keycode) const
	{
		// handle multichar input sequences specially
		if (keycode == MULTICHAR_SEQUENCE)
			return Control::SEQUENCE;

		for (const auto& [ctrl, keys] : bindings)
			if (keys(keycode))
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
				const auto val{ ini.getvs_cast<KeyBind>("controls", keyName, [](std::string&& s)->KeyBind {return KeyBind{ std::move(s) }; }) };
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
			ini.set("controls", "key_up", bindings.at(Control::UP).keyCodes);
			ini.set("controls", "key_down", bindings.at(Control::DOWN).keyCodes);
			ini.set("controls", "key_left", bindings.at(Control::LEFT).keyCodes);
			ini.set("controls", "key_right", bindings.at(Control::RIGHT).keyCodes);
			ini.set("controls", "key_pause", bindings.at(Control::PAUSE).keyCodes);
			ini.set("controls", "key_restart", bindings.at(Control::RESTART).keyCodes);
			ini.set("controls", "key_quit", bindings.at(Control::QUIT).keyCodes);
			return ini;
		}
	};
