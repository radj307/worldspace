#pragma once
#include <atomic>	// for thread-safe variables
#include <chrono>
#include <iostream>
#include <optional>
#include <string>

#include "Coord.h"

namespace game::_internal {
	using namespace std::chrono_literals; // for time literals
	static const int
		PLAYER_WIN_CODE{ 1 },     ///< Player wins when this code is set
		PLAYER_LOSE_CODE{ 0 },    ///< Player loses when this code is set
		PLAYER_QUIT_CODE{ -1 },   ///< Player quit when this code is set
		GAME_EXCEPTION_CODE{ 2 }; ///< An exception was thrown

	/**
	 * calcFrametime(unsigned int)
	 * @brief Calculates the frametime (time between frames) needed to achieve a given target framerate
	 * @param fps	- Framerate in frames per second
	 * @returns chrono::milliseconds
	 */
	constexpr auto calcFrametime(const unsigned int fps) { return std::chrono::milliseconds(1000) / fps; }

	static unsigned int __FRAMERATE; ///< Target framerate, aka display cycles per second
	static std::chrono::duration<float>
		__FRAMETIME, ///< Target frametime, aka display cycle delay
		__NPC_CLOCK; ///< Target NPC Cycle, aka NPC action delay.

	/**
	 * setFramerate(unsigned int)
	 * @brief Set the target number of frames per second.
	 * @param newFramerate	- In frames per second
	 */
	inline bool setFramerate(const unsigned int newFramerate)
	{
		try {
			__FRAMERATE = newFramerate;
			__FRAMETIME = calcFrametime(__FRAMERATE);
			return true;
		} catch (...) {
			__FRAMERATE = 1;
			__FRAMETIME = 100ms;
			return false;
		}
	}

	/**
	 * setNPCCycle(unsigned int)
	 * @brief Set the amount of time between each NPC action cycle.
	 * @param cycleTimeMS	- Time between cycles in milliseconds
	 */
	inline bool setNPCCycle(const unsigned int cycleTimeMS)
	{
		try {
			__NPC_CLOCK = std::chrono::milliseconds{ cycleTimeMS };
			return true;
		} catch (...) {
			__NPC_CLOCK = 100ms;
			return false;
		}
	}

	/**
	 * @brief Contains atomic variables shared between all threads
	 */
	struct memory {
		std::atomic<bool>
			_kill{ false }, ///< true = kill all threads
			_pause{ false }, ///< true = pause all threads
			_pause_complete{ false }; ///< true = display has been de-initialized
		std::atomic<int> _kill_code{ -2 }; ///< -2 = not set | see the above PLAYER_CODE vars.
		std::optional<std::string> _player_killed_by{ std::nullopt };
		const std::string _pause_msg{ "GAME PAUSED" };
		///< This is an optional string used to display the name of the actor who killed the player. This is only set by the game::start() function

		/**
		 * pause_game(Coord)
		 * @brief Sets the pause flag, clears the screen, and prints the pause message to a given pos.
		 * @param textPos	- Location in the screen buffer to display the pause message
		 */
		void pause_game(const Coord textPos = Coord(5, 3))
		{
			_pause.store(true);
			sys::term::cls();
			sys::term::cursorPos(textPos);
			std::cout << Color::f_cyan << _pause_msg << Color::reset;
		}

		/**
		 * unpause_game()
		 * @brief Disables the pause flag and clears the screen
		 */
		void unpause_game()
		{
			sys::term::cls();
			_pause.store(false);
		}
	};
}
