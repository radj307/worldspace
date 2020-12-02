/**
 * threaded.h
 * This file contains all of the game-operation logic, including all threads & controllers.
 * by radj307
 */
#pragma once
#include <atomic>	// for thread-safe variables
#include <future>	// for capturing thread return values
#include <mutex>	// for mutexes & scoped locks

#include "FrameBuffer.h"
#include "sys.h"

static const std::chrono::milliseconds __MS_CLOCK_SYNC{ 30 }; // synchronizes the amount of time each thread sleeps
static const unsigned int __NPC_CYCLE_WAIT_MULT{ 8 }; // each NPC action occurs every clock cycle multiplied by this
static const char __BLANK_KEY = '\\'; // key press that does not perform any player action. Used to reset key presses.

// Shared Memory Object
struct memory {
	std::atomic<char> key{ __BLANK_KEY };			// player's last key press
	std::atomic<bool> kill{ false };				// thread kill flag
	std::atomic<bool> pause{ false };				// pause game flag
	std::atomic<int> win{ -2 };
	const std::string pause_msg{ "GAME PAUSED" };	// Message to show when paused

	// Resets the key var to default (unpressed) state.
	void reset_key()
	{
		key.store(__BLANK_KEY);
	}
};
// instantiate shared memory
inline memory mem;

// create a mutex to prevent critical section overlap
inline std::mutex mutx;

/**
 * game_thread_player(Gamespace&, GLOBAL&)
 * Processes player input
 * Loops until the shared memory's kill flag is true
 *
 * @param game	- Reference to the associated gamespace, passed with std::ref()
 */
inline void game_thread_player(Gamespace& game)
{
	for (const auto input{__BLANK_KEY}; !mem.kill.load(); ) {
		// getch waits until key press, no need to sleep this thread.
		mem.key.store(static_cast<char>(_getch()));
		// if game is not paused
		if ( !mem.pause.load() ) {
			// switch player keypress
			switch ( mem.key.load() ) {
			case __BLANK_KEY:break; // player has not pressed a key since last key was processed
			case 'q': // player pressed the exit game key
				mem.kill.store(true);
				//std::cout << std::endl << std::endl;
				//sys::msg(sys::log, "Kill request received. Shutting down.");
				return;
			case 'p': // player pressed the pause game key
				mem.pause.store(true);
				std::this_thread::sleep_for(__MS_CLOCK_SYNC); // wait until the game_thread_display function is 100% paused
				sys::cls();
				sys::setCursorPos(5, 3);
				std::cout << termcolor::cyan << mem.pause_msg << termcolor::reset;
				break;
			default: // player pressed a different key, process it
				std::scoped_lock<std::mutex> lock(mutx); // lock the mutex
				switch ( game.actionPlayer(mem.key.load()) ) {
				case 0:
					mem.kill.store(true);
					return;
				case 2:
					mem.kill.store(true);
					mem.win.store(0);
					return;
				default:break;
				}
				mem.reset_key();
				break;
			}
		} // else check if player wants to unpause
		else if ( mem.key.load() == 'p' ) { mem.pause.store(false); sys::cls(); }
		// else do nothing
	}
//	return 2;
}
/**
 * game_thread_npc(Gamespace&)
 * Processes non-player turns.
 * Loops until the shared memory's kill flag is true
 *
 * @param game	- Reference to the associated gamespace, passed with std::ref()
 */
inline void game_thread_npc(Gamespace& game)
{
	// loop until shared memory's kill flag is true
	while ( !mem.kill.load() ) {
		std::this_thread::sleep_for(__MS_CLOCK_SYNC * __NPC_CYCLE_WAIT_MULT);
		if ( !mem.pause.load() ) {
			game.actionAllNPC();
			//game.actionHostile();
		}
		else std::this_thread::sleep_for(std::chrono::seconds(1) - __MS_CLOCK_SYNC * __NPC_CYCLE_WAIT_MULT);
	}
}
/**
 * game_thread_display(Gamespace&)
 * Displays the gamespace to the console every half clock cycle
 * Loops until the shared memory's kill flag is true
 *
 * @param game	- Reference to the associated gamespace, passed with std::ref()
 */
inline void game_thread_display(Gamespace& game)
{
	typedef std::chrono::high_resolution_clock T;
	// create a frame buffer with the given gamespace ref
	FrameBuffer_Gamespace gameBuffer(game, Coord(3, 3));
	for (auto tLastRegenCycle{ T::now() }; !mem.kill.load(); ) {
		if ( !mem.pause.load() ) {
			std::this_thread::sleep_for(__MS_CLOCK_SYNC / 2);
			// flush the cout buffer
			std::cout.flush();
			// lock the mutex
			std::scoped_lock<std::mutex> lock(mutx);
			// display the gamespace
			gameBuffer.display();
			// Apply passive effects every second
			if ( T::now() - tLastRegenCycle >= std::chrono::seconds(1) ) {
				game.apply_passive();
				tLastRegenCycle = T::now();
			}
			// Check if the player won
			if ( game._allEnemiesDead ) {
				mem.win.store(0);
				mem.kill.store(true);
				return;
			}
			// check if the player died
			else if ( game.getPlayer().isDead() ) {
				mem.win.store(-1);
				mem.kill.store(true);
				return;
			}
		}
		else { // the game is paused
			// set the frame-buffer's initialized flag to false, causing the frame to be reinitialized on unpause
			gameBuffer.deinitialize();
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
}

/**
 * process_game_over(int)
 * Prints a game over message to the console with win/lose status.
 *
 * @param winStateCode	- ( -1 = Player Loses ) ( 0 = Player Wins ) ( 1 = Player exited ) ( Other = Undefined )
 */
inline void process_game_over(const int winStateCode)
{
	sys::cls();
	switch ( winStateCode ) {
	case -1: // player lost the game
		std::cout << termcolor::red << "You lost!" << termcolor::reset << std::endl;
		break;
	case 0: // player won the game
		std::cout << termcolor::green << "You won!" << termcolor::reset << std::endl;
		break;
	case 1: // player exited the game
		std::cout << termcolor::cyan << "Game Over." << termcolor::reset << std::endl;
		break;
	default: // undefined
		std::cout << termcolor::red << "The game did not shut down correctly, exiting...." << termcolor::reset << std::endl;
		break;
	}
	sys::sleep(1000);
	return;
}

inline void game_start(GLOBAL settings)
{
	GameRules rules; // define ruleset
	Gamespace thisGame(settings, rules); // create the gamespace

	std::thread player(&game_thread_player, std::ref(thisGame));
	std::thread display(&game_thread_display, std::ref(thisGame));
	std::thread enemy(&game_thread_npc, std::ref(thisGame));

	player.join();
	display.join();
	enemy.join();

	process_game_over(mem.win.load());
}