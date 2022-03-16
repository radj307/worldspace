/**
 * @file	GameOverMenu.hpp
 * @author	radj307
 * @brief	Contains the game over menu's display functions.
 */
#pragma once
#include "positionable_text.h"
#include "../Controls.h"

#include <strconv.hpp>

struct GameOverMenu : positionable_text {
	std::vector<std::vector<std::string>> text;
	Controls& ctrls;

	GameOverMenu(const point& csbTopMiddle, Controls& ctrls) : positionable_text(csbTopMiddle, text), ctrls{ ctrls }
	{
		text = std::vector<std::vector<std::string>>{
			std::vector<std::string>{ "Game Over!" },
		};
		text.emplace_back(std::vector<std::string>{""});
		{
			text.emplace_back(std::vector<std::string>{ "Press <" });
			auto& v{ text.back() };
			const auto& restart_keys{ ctrls.getKeyBindsFor(Control::RESTART) };
			for (auto it{ restart_keys.begin() }, endit{ restart_keys.end() }; it != endit; ++it) {
				v.emplace_back(color::setcolor::green);
				v.emplace_back(str::ctos(*it));
				v.emplace_back(color::setcolor::reset);
				if (std::distance(it, restart_keys.end()) > 1)
					v.emplace_back(", ");
			}
			v.emplace_back("> to restart.");
		}
		text.emplace_back(std::vector<std::string>{"or"});
		{
			text.emplace_back(std::vector<std::string>{"Press <"});
			auto& v{ text.back() };
			const auto& quit_keys{ ctrls.getKeyBindsFor(Control::QUIT) };
			for (auto it{ quit_keys.begin() }, endit{ quit_keys.end() }; it != endit; ++it) {
				v.emplace_back(color::setcolor::red);
				v.emplace_back(str::ctos(*it));
				v.emplace_back(color::setcolor::reset);
				if (std::distance(it, quit_keys.end()) > 1)
					v.emplace_back(", ");
			}
			v.emplace_back("> to quit.");
		}
	}

	void display()
	{
		std::cout << term::clear << *this;
	}
};
