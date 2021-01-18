#pragma once
#include <atomic>	// for thread-safe variables
#include <chrono>
#include <iostream>
#include <optional>
#include <string>

#include "Coord.h"
#include "termcolor/termcolor.hpp"

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
		} catch ( ... ) {
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
		} catch ( ... ) {
			__NPC_CLOCK = 100ms;
			return false;
		}
	}

	/**
	 * @brief Contains atomic variables shared between all threads
	 */
	struct memory {
	private:
		// Default values are set here
		const bool 
			_DEF_KILL{ false }, ///< Default kill flag value
			_DEF_PAUSE{ false }, ///< Default pause flag value
			_DEF_PAUSE_COMPLETE{ false }; ///< Default pause_complete flag value
		const int _DEF_KILL_CODE{ -2 }; ///< Default kill_code value
		const std::string pause_msg{ "GAME PAUSED" };
		///< This is the message displayed to the screen while the game is paused

	public:
		std::atomic<bool>
			kill{ _DEF_KILL }, ///< true = kill all threads
			pause{ _DEF_PAUSE }, ///< true = pause all threads
			pause_complete{ _DEF_PAUSE_COMPLETE }; ///< true = display has been de-initialized
		std::atomic<int> kill_code{ _DEF_KILL_CODE }; ///< -2 = not set | see the above PLAYER_CODE vars.
		std::optional<std::string> player_killed_by{ std::nullopt };
		///< This is an optional string used to display the name of the actor who killed the player. This is only set by the game::start() function
		
		/**
		 * pause_game(Coord)
		 * @brief Sets the pause flag, clears the screen, and prints the pause message to a given pos.
		 * @param textPos	- Location in the screen buffer to display the pause message
		 */
		void pause_game(const Coord textPos = Coord(5, 3))
		{
			pause.store(true);
			sys::cls();
			sys::cursorPos(textPos);
			std::cout << termcolor::cyan << pause_msg << termcolor::reset;
		}

		/**
		 * unpause_game()
		 * @brief Disables the pause flag and clears the screen
		 */
		void unpause_game()
		{
			sys::cls();
			pause.store(false);
		}

		/**
		 * reset_all()
		 * Resets all variables to their original state.
		 */
		void reset_all()
		{
			kill.store(_DEF_KILL);
			pause.store(_DEF_PAUSE);
			pause_complete.store(_DEF_PAUSE_COMPLETE);
			kill_code.store(_DEF_KILL_CODE);
		}
	};
}