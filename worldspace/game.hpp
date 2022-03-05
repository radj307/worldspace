/**
 * threaded.h
 * This file contains all of the game-operation logic, including all threads & controllers.
 * @author radj307
 */
#pragma once
#include "Coord.h"
#include "init.h"
#include "shared.h"
#include "ThreadFunctions.h"

#include <future>	// for capturing thread return values
#include <INI.hpp>	// for INI parser
#include <mutex>	// for mutexes & scoped locks
#include <sys_legacy/sys.h>	// for system commands
#include <thread>	// for threads

 /**
  * @namespace game
  * @brief Contains multithread functions & handlers for running a game. This is the only interaction point between main() and the game.
  */
namespace game {
	/**
	 * @namespace _internal
	 * @brief Contains high-level game functions that should not be accessed outside of the game namespace.
	 */
	namespace _internal {

		/**
		 * process_game_over(memory&, Coord)
		 * @brief Clears the screen & prints an informational game over message to the terminal.
		 * @param mem		- Shared memory instance ref, checks members kill_code & player_killed_by
		 * @param textPos	- Position in the screen buffer to print message.
		 */
		inline void print_game_over(memory& mem, const std::optional<Coord>& textPos = std::nullopt)
		{
			const auto pos{ textPos.has_value() ? sys::iCOORD(textPos.value()._x, textPos.value()._y) : sys::iCOORD{ sys::getScreenBufferCenter()._x - 9, 10 } };
			sys::cls(true);
			sys::cursorPos(pos._x, pos._y);
			switch (mem._kill_code.load()) {
			case PLAYER_LOSE_CODE: // player lost the game
				[&pos](const std::optional<std::string>& killer) {
					const std::string
						loseText{ "You lost!" },
						killerText{ "killed by: " + killer.value_or("") };
					std::cout << color::f::red << loseText << color::reset;
					if (killer.has_value()) {
						sys::cursorPos(pos._x - killerText.size() / 2u + loseText.size() / 2, pos._y + 1);
						std::cout << color::f::red << killerText << color::reset;
					}
				}(mem._player_killed_by);
				break;
			case PLAYER_WIN_CODE: // player won the game
				std::cout << color::f::green << "You won!" << color::reset;
				break;
			case PLAYER_QUIT_CODE: // player exited the game
				std::cout << color::f::cyan << "Game Over." << color::reset;
				break;
			case GAME_EXCEPTION_CODE:
			default: // undefined
				std::cout << color::f::red << "The game exited with an undefined error." << color::reset;
				break;
			}
			std::this_thread::sleep_for(500ms);
			printf("%s", std::string(20, '\n').c_str());
		}
	} // namespace _internal

	/**
	 * start(vector<string>&, optional<CONTROLS>, optional<GameRules>)
	 * @brief Thread Manager. Starts the game threads and returns once the game is over.
	 * @param INI_Files		- String vector containing INI filenames.
	 * @param controlset	- (Default: nullopt) Optional controlset override, including this will disable loading the controlset from INI.
	 * @param ruleset		- (Default: nullopt) Optional ruleset override, including this will disable loading the ruleset from INI.
	 * @return true			- Game exited because it was over.
	 * @return false		- An exception was thrown, or the Player quit the game.
	 */
	inline bool start(const std::vector<std::string>& INI_Files, const std::optional<CONTROLS>& controlset = std::nullopt, const std::optional<GameRules>& ruleset = std::nullopt)
	{
		// read from INI file
		if (!file::exists("def.ini"))
			_internal::initDefaultINI("def.ini");
		file::INI cfg;// (INI_Files);
		for (auto& it : INI_Files)
			cfg.read(it);
		cfg.read("def.ini");

		try {
			_internal::initTiming(cfg);                      ///< Initialize the clock timings
		} catch (...) { return false; }
		auto controls{ controlset.value_or(_internal::initControlSet(cfg)) }; ///< Initialize the controlset
		auto rules{ ruleset.value_or(_internal::initRuleset(cfg)) };       ///< Initialize the ruleset

		_current_control_set = &controls; ///< Set the current control set ptr

		// instantiate shared memory
		_internal::memory mem;

		// create a mutex to prevent critical section overlap
		std::mutex mutx;

		// Create gamespace with ruleset
		Gamespace thisGame(rules);

		try { // Start the game threads
			auto // Init asynchronous threads
				display [[maybe_unused]] { std::async(std::launch::async, &_internal::thread_display, std::ref(mutx), std::ref(mem), std::ref(thisGame), std::ref(rules)) },
				enemy [[maybe_unused]] { std::async(std::launch::async, &_internal::thread_npc, std::ref(mutx), std::ref(mem), std::ref(thisGame)) },
				player [[maybe_unused]] { std::async(std::launch::async, &_internal::thread_player, std::ref(mutx), std::ref(mem), std::ref(thisGame)) };
		} catch (std::exception& ex) {
			sys::cls();
			std::cout << sys::term::error << "An unhandled thread exception occurred, but was caught by the thread manager: \"" << ex.what() << "\"" << std::endl;
		}
		if (mem._kill_code.load() == _internal::PLAYER_LOSE_CODE)
			mem._player_killed_by = thisGame.getPlayer().killedBy();
		// Once the kill flag is true, show the game over message and return
		print_game_over(mem);
		// Check if the restart prompt should be shown, and return
		return mem._kill_code.load() != _internal::PLAYER_QUIT_CODE;
	}
}