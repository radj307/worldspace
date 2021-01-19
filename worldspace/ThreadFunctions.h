/**
 * @file ThreadFunctions.h
 * @author radj307
 * @brief Contains main functions for multithreads from the game namespace. \n
 * Used in game_threads.hpp
 */
#pragma once
#pragma region THREAD_FUNC
#include <conio.h>
#include <mutex>

#include "controls.h"
#include "FrameBuffer.h"
#include "Gamespace.h"
#include "shared.h"

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
	inline void thread_player( std::mutex& mutx, memory& mem, Gamespace& game )
	{
		const auto end_game { [&mem](const int KILL_CODE) {
			mem.kill_code.store(KILL_CODE);
			mem.kill.store(true);

		} };
		while ( !mem.kill.load() ) {
			// getch waits until key press, no need to sleep this thread.
			if ( _kbhit() ) {
				const auto key{ static_cast<char>(std::tolower( _getch() )) };
				// if game is not paused
				if ( !mem.pause.load() ) {
					if ( key == _current_control_set->_KEY_QUIT ) {

						return;
					}
					// switch player keypress
					switch ( key ) {
					case 'q': // player pressed the exit game key
						mem.kill_code.store( PLAYER_QUIT_CODE );
						game._game_state._game_is_over.store( true );
						mem.kill.store( true );
						return;
					case 'p': // player pressed the pause game key
						mem.pause.store( true );
						break;
					default: // player pressed a different key, process it
						std::scoped_lock<std::mutex> game_lock( mutx ); // lock the mutex
						game.actionPlayer( key );
						break;
					}
				} // else check if player wants to unpause
				else if ( key == 'p' ) 
					mem.unpause_game();
			}
			else
				std::this_thread::sleep_for( __FRAMETIME );
		}
	}

	/**
	 * game_thread_npc(Gamespace&)
	 * @brief Thread function that controls the NPC actors.
	 * @param mutx	- Shared Mutex
	 * @param mem	- Shared Memory
	 * @param game	- Reference to the associated gamespace, passed with std::ref()
	 */
	inline void thread_npc( std::mutex& mutx, memory& mem, Gamespace& game )
	{
		// loop until shared memory's kill flag is true
		while ( !mem.kill.load() ) {
			std::this_thread::sleep_for( __NPC_CLOCK );
			if ( !mem.pause.load() ) {
				std::scoped_lock<std::mutex> game_lock( mutx );	// lock the mutex
				game.actionAllNPC();						// perform NPC actions
			}
			else std::this_thread::sleep_for(1s);
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
	inline void thread_display( std::mutex& mutx, memory& mem, Gamespace& game, GameRules& cfg )
	{
		// create a frame buffer with the given gamespace ref
		FrameBuffer gameBuffer( game, Coord( 1920 / 3, 1080 / 8 ) );
		// Loop until kill flag is true
		for ( auto tLastRegenCycle{ CLK::now() }; !mem.kill.load(); ) {
			if ( !mem.pause.load() ) {
				mem.pause_complete.store( false );
				std::this_thread::sleep_for( __FRAMETIME );
				std::scoped_lock<std::mutex> display_lock( mutx );
				try {
					gameBuffer.display();
				} catch ( std::exception& ) {
			//		std::stringstream ss{ std::string(" Exception thrown during display call: ") + ex.what() };
			//		file::write("");
				}

				game.apply_level_ups();
				if ( game._game_state._game_is_over.load() ) {
					mem.kill.store( true );
					if ( game._game_state._playerDead.load() )
						mem.kill_code.store( PLAYER_LOSE_CODE );
					else if ( game._game_state._allEnemiesDead.load() )
						mem.kill_code.store( PLAYER_WIN_CODE );
					break;
				}
				else if ( CLK::now() - tLastRegenCycle >= cfg._regen_timer ) {
					game.apply_passive();
					tLastRegenCycle = CLK::now();
				}
			}
			else if ( !mem.pause_complete.load() ) {
				gameBuffer.deinitialize();
				mem.pause_game();
				mem.pause_complete.store( true );
			}
			else
				std::this_thread::sleep_for( __FRAMETIME );
		}
	}
}
