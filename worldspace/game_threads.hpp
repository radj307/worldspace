/**
 * threaded.h
 * This file contains all of the game-operation logic, including all threads & controllers.
 * by radj307
 */
#pragma once
#include <atomic>	// for thread-safe variables
#include <future>	// for capturing thread return values
#include <mutex>	// for mutexes & scoped locks
#include <thread>	// for threads

#include "FrameBuffer.h"
#include "sys.h"

namespace game {
	namespace _internal {
		constexpr int PLAYER_WIN_CODE{ 1 }; // Player wins when this code is set
		constexpr int PLAYER_LOSE_CODE{ 0 }; // Player loses when this code is set
		constexpr int PLAYER_QUIT_CODE{ -1 }; // Player quit when this code is set
		constexpr char __BLANK_KEY{ '\\' }; // Unused key, for preventing input loops
		constexpr std::chrono::milliseconds __MS_CLOCK_SYNC{ 15 }; // Frametime
		constexpr std::chrono::milliseconds __NPC_CYCLE_WAIT_MULT{ __MS_CLOCK_SYNC * 13 }; // How long to wait between enemy actions

		// Shared Memory Object
		struct memory {
		private:
			// This is the message displayed to the screen while the game is paused
			const std::string pause_msg{ "GAME PAUSED" };

		public:
			std::atomic<bool>
				kill{ false }, // Used by all threads to determine when to shutdown
				pause{ false }, // Used by all threads to determine when the game is paused
				pause_complete{ false },
				player_exit{ false },
				player_reinit{ false };

			std::atomic<int> kill_code{ -2 };

			// To be called from the display thread
			void pause_game(const Coord textPos = Coord(5, 3))
			{
				pause.store(true);
				WinAPI::cls();
				WinAPI::setCursorPos(textPos);
				std::cout << termcolor::cyan << pause_msg << termcolor::reset;
			}
			// To be called from the player input thread
			void unpause_game()
			{
				WinAPI::cls();
				pause.store(false);
			}

			/**
			 * reset_all()
			 * Resets all variables to their original state
			 */
			void reset_all()
			{
				kill.store(false);
				pause.store(false);
				kill_code.store(-2);
			}
		};

		/**
		 * process_game_over(int)
		 * Prints a game over message to the console with win/lose status.
		 * Called from display thread.
		 *
		 * @param winStateCode	- ( -1 = Player Loses ) ( 0 = Player Wins ) ( 1 = Player exited ) ( Other = Undefined )
		 * @param textPos		- Position in the screen buffer to print message.
		 */
		inline void print_game_over(const int winStateCode, const Coord textPos = Coord(9, 3))
		{
			WinAPI::cls();
			WinAPI::setCursorPos(textPos);
			switch ( winStateCode ) {
			case PLAYER_LOSE_CODE: // player lost the game
				std::cout << termcolor::red << "You lost!" << termcolor::reset;
				break;
			case PLAYER_WIN_CODE: // player won the game
				std::cout << termcolor::green << "You won!" << termcolor::reset;
				break;
			case PLAYER_QUIT_CODE: // player exited the game
				std::cout << termcolor::cyan << "Game Over." << termcolor::reset;
				break;
			default: // undefined
				std::cout << termcolor::red << "The game exited with an undefined error." << termcolor::reset;
				break;
			}
			sys::sleep(500);
		}

		/**
		 * game_thread_player(Gamespace&, GLOBAL&)
		 * Processes player input
		 * Loops until the shared memory's kill flag is true
		 *
		 * @param mutx	- Shared Mutex
		 * @param mem	- Shared Memory
		 * @param game	- Reference to the associated gamespace, passed with std::ref()
		 */
		inline void thread_player(std::mutex& mutx, memory& mem, Gamespace& game)
		{
			while ( !mem.kill.load() ) {
				// getch waits until key press, no need to sleep this thread.
				if ( _kbhit() ) {
					const auto key{ static_cast<char>(_getch()) };
					//mem.key.store(static_cast<char>(_getch()));

					// if game is not paused
					if ( !mem.pause.load() ) {
						// switch player keypress
						switch ( key ) {
						case __BLANK_KEY:break; // player has not pressed a key since last key was processed
						case 'q': // player pressed the exit game key
							mem.kill_code.store(PLAYER_QUIT_CODE);
							mem.player_exit.store(true); // set the quit flag to prevent restart prompt
							mem.kill.store(true);
							return;
						case 'p': // player pressed the pause game key
							mem.pause.store(true);
							break;
						default: // player pressed a different key, process it
							std::scoped_lock<std::mutex> lock(mutx); // lock the mutex
							game.actionPlayer(key);
						//	mem.reset_key();
							break;
						}
					} // else check if player wants to unpause
					else if ( key == 'p' ) { mem.unpause_game(); }
				}
				else std::this_thread::sleep_for(__MS_CLOCK_SYNC);
			}
		}
		/**
		 * game_thread_npc(Gamespace&)
		 * Processes non-player turns.
		 * Loops until the shared memory's kill flag is true
		 *
		 * @param mutx	- Shared Mutex
		 * @param mem	- Shared Memory
		 * @param game	- Reference to the associated gamespace, passed with std::ref()
		 */
		inline void thread_npc(std::mutex& mutx, memory& mem, Gamespace& game)
		{
			// loop until shared memory's kill flag is true
			while ( !mem.kill.load() ) {
				std::this_thread::sleep_for(__NPC_CYCLE_WAIT_MULT);
				if ( !mem.pause.load() ) {
					std::scoped_lock<std::mutex> lock(mutx);	// lock the mutex
					game.actionAllNPC();						// perform NPC actions
				}
				else std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		}
		/**
		 * game_thread_display(Gamespace&)
		 * Displays the gamespace to the console every half clock cycle
		 * Loops until the shared memory's kill flag is true
		 *
		 * @param mutx	- Shared Mutex
		 * @param mem	- Shared Memory
		 * @param game	- Reference to the associated gamespace, passed with std::ref()
		 * @param cfg	- Game Rules
		 */
		inline void thread_display(std::mutex& mutx, memory& mem, Gamespace& game, GameRules& cfg)
		{
			typedef std::chrono::high_resolution_clock T;

			// create a frame buffer with the given gamespace ref
			FrameBuffer gameBuffer(game, Coord(3, 3), Coord(1920, 5));

			// Loop until kill flag is true
			for ( auto tLastRegenCycle{ T::now() }; !mem.kill.load(); ) {
				if ( !mem.pause.load() ) {
					mem.pause_complete.store(false);
					std::this_thread::sleep_for(__MS_CLOCK_SYNC);		// sleep this thread for half of a clock cycle
					std::cout.flush();									// flush the cout buffer
					std::scoped_lock<std::mutex> lock(mutx);			// lock the mutex
					game.apply_level_ups();								// Apply level ups
					gameBuffer.display();								// display the gamespace
					if ( T::now() - tLastRegenCycle >= cfg._regen_timer ) {
						game.apply_passive();							// Apply passive effects every second
						tLastRegenCycle = T::now();
					}
					if ( game.playerWon() ) {							// Check if all enemies are dead
						mem.kill_code.store(PLAYER_WIN_CODE);			// Send win code
						mem.kill.store(true);							// Send kill code
						break;											// break loop
					}
					if ( game.playerLost() ) {							// check if the player died
						mem.kill_code.store(PLAYER_LOSE_CODE);			// Send lose code
						mem.kill.store(true);							// Send kill code
						break;											// break loop
					}
				}
				else if ( !mem.pause_complete.load() ) {
					gameBuffer.deinitialize();
					mem.pause_game();
					mem.pause_complete.store(true);
				}
				else std::this_thread::sleep_for(__MS_CLOCK_SYNC);
			}
			// Once the kill flag is true, show the game over message and return
			print_game_over(mem.kill_code.load());
		}
	}

	/**
	 * game_start(GLOBAL&)
	 * Starts the game threads and returns once the game is over.
	 *
	 * @param settings	- The list of global settings parsed from the commandline
	 * @returns bool	- ( true = player pressed the quit button ) ( false = game exited normally )
	 */
	inline bool start(const GLOBAL& settings)
	{
		// instantiate shared memory
		_internal::memory mem;

		// create a mutex to prevent critical section overlap
		std::mutex mutx;

		// Create ruleset
		GameRules rules(settings);

		// Create gamespace
		Gamespace thisGame(rules);

		try {
			auto // Start asynchronous threads
				display{ std::async(std::launch::async, &_internal::thread_display, std::ref(mutx), std::ref(mem), std::ref(thisGame), std::ref(rules)) },
				enemy{ std::async(std::launch::async, &_internal::thread_npc, std::ref(mutx), std::ref(mem), std::ref(thisGame)) },
				player{ std::async(std::launch::async, &_internal::thread_player, std::ref(mutx), std::ref(mem), std::ref(thisGame)) };
		} catch ( std::exception & ex ) {
			msg(sys::error, "\"" + std::string(ex.what()) + "\" was thrown while starting the game threads.", "Press any key to exit....");
		}
		// Return true if the player pressed the quit button
		return mem.player_exit.load();
	}
}