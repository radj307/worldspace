/**
 * @file ThreadFunctions.h
 * @author radj307
 * @brief Contains main functions for multithreads from the game namespace. \n
 * Used in game_threads.hpp
 */
#pragma once
#include "FrameBuffer.h"
#include "Gamespace.h"
#include "shared.h"

 //#include <conio.h>
#include <cstdio>
#include <mutex>
#include <hasPendingDataSTDIN.h>

/**
 * @namespace _internal
 * @brief Contains high-level game functions that should not be accessed outside of the game namespace.
 */
namespace game::_internal {
	using CLK = std::chrono::high_resolution_clock; ///< Game timer clock

	/**
	 * game_thread_player(Gamespace&, GLOBAL&)
	 * @brief Thread function that receives & processes player key presses independantly of the display/npc threads.
	 * @param mutx	- Shared Mutex
	 * @param mem	- Shared Memory
	 * @param game	- Reference to the associated gamespace, passed with std::ref()
	 */
	inline void thread_player(std::mutex& mutx, memory& mem, Gamespace& game)
	{
		try {
			while (!mem._kill.load()) {
				//	if (mem._exception.has_value())
				//		return;
				// getch waits until key press, no need to sleep this thread.
				if (term::kbhit()) {
					const auto key{ static_cast<char>(std::tolower(term::getch())) };
					// if game is not paused
					if (!mem._pause.load()) {
						// switch player keypress
						switch (key) {
						case 'q': // player pressed the exit game key
							mem._kill_code.store(PLAYER_QUIT_CODE);
							game._game_state._game_is_over.store(true);
							mem._kill.store(true);
							return;
						case 'p': // player pressed the pause game key
							mem._pause.store(true);
							break;
						default: // player pressed a different key, process it
							std::scoped_lock<std::mutex> game_lock(mutx); // lock the mutex
							game.actionPlayer(key);
							break;
						}
					} // else check if player wants to unpause
					else if (key == 'p')
						mem.unpause_game();
				}
				else std::this_thread::sleep_for(__FRAMETIME);
			}
		} catch (const std::exception& ex) {
			mem._exception = ex;
			return;
		}
	}

	/**
	 * game_thread_npc(Gamespace&)
	 * @brief Thread function that controls the NPC actors.
	 * @param mutx	- Shared Mutex
	 * @param mem	- Shared Memory
	 * @param game	- Reference to the associated gamespace, passed with std::ref()
	 */
	inline void thread_npc(std::mutex& mutx, memory& mem, Gamespace& game)
	{
		try {
			// loop until shared memory's kill flag is true
			while (!mem._kill.load()) {
				std::this_thread::sleep_for(__NPC_CLOCK);
				if (!mem._pause.load()) {
					std::scoped_lock<std::mutex> game_lock(mutx);	// lock the mutex
					game.actionAllNPC();						// perform NPC actions
				}
				else std::this_thread::sleep_for(1s);
			}
		} catch (const std::exception& ex) {
			mem._exception = ex;
			return;
		}
	}

	/**
	 * game_thread_display(Gamespace&)
	 * @brief Thread function that controls the display.
	 * @param mutx	- Shared Mutex
	 * @param mem	- Shared Memory
	 * @param game	- Reference to the associated gamespace, passed with std::ref()
	 * @param cfg	- Game Rules
	 */
	inline void thread_display(std::mutex& mutx, memory& mem, Gamespace& game, GameRules& cfg)
	{
		try {
			// create a frame buffer with the given gamespace ref
			FrameBuffer gameBuffer(game, Coord(1920 / 3, 1080 / 8));
			// Loop until kill flag is true
			for (auto tLastRegenCycle{ CLK::now() }; !mem._kill.load(); ) {
			//	if (mem._exception.has_value())
			//		return;
				if (!mem._pause.load()) {
					mem._pause_complete.store(false);
					std::this_thread::sleep_for(__FRAMETIME);
					std::scoped_lock<std::mutex> display_lock(mutx);
					try {
						gameBuffer.display();
					} catch (std::exception&) {}

					game.apply_level_ups();
					if (game._game_state._game_is_over.load()) {
						mem._kill.store(true);
						if (game._game_state._playerDead.load())
							mem._kill_code.store(PLAYER_LOSE_CODE);
						else if (game._game_state._allEnemiesDead.load())
							mem._kill_code.store(PLAYER_WIN_CODE);
						break;
					}
					// ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
					else if (CLK::now() - tLastRegenCycle >= cfg._regen_timer) {
						game.apply_passive();
						tLastRegenCycle = CLK::now();
					}
				}
				else if (!mem._pause_complete.load()) {
					gameBuffer.deinitialize();
					mem.pause_game();
					mem._pause_complete.store(true);
				}
				else
					std::this_thread::sleep_for(__FRAMETIME);
			}
		} catch (const std::exception& ex) {
			mem._exception = ex;
			return;
		}
	}
}
