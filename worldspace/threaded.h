/**
 * threaded.h
 * This file contains all of the game-operation logic, including all threads & controllers.
 * by radj307
 */
#pragma once
#include <mutex>
#include <atomic>	// for thread-safe variables
#include <future>
#include "FrameBuffer.h"
#include "sys.h"

static const std::chrono::milliseconds __MS_CLOCK_SYNC{ 30 }; // synchronizes the amount of time each thread sleeps
static const unsigned int __NPC_CYCLE_WAIT_MULT{ 8 }; // each NPC action occurs every clock cycle multiplied by this
static const char __BLANK_KEY = '\\'; // key press that does not perform any player action. Used to reset key presses.
// wrapper for atomic vars
struct memory {
	std::atomic<char> key{ __BLANK_KEY };	// player's last key press
	std::atomic<bool> kill{ false };		// thread kill flag
	std::atomic<bool> pause{ false };		// pause game flag
	const std::string pause_msg{ "GAME PAUSED" };

	inline void reset_key()
	{
		key.store(__BLANK_KEY);
	}
};
// instantiate shared memory
memory mem;

// create a mutex to prevent actor actions during the frame draw cycle
std::mutex mutx;

// Threaded functions:
/**
 * play(Gamespace&, GLOBAL&)
 * Processes player input
 * Loops until the shared memory's kill flag is true
 *
 * @param game	- Reference to the associated gamespace, passed with std::ref()
 * @param g		- Reference to the global settings, passed with std::ref()
 * @returns int	- ( 1 = Player wants to exit ) ( -1 = Kill flag received from another source, such as if player died. )
 */
int play(Gamespace& game, GLOBAL& g)
{
	for ( char input{}; input != 'q' && !mem.kill.load(); ) {
		// getch waits until key press, no need to sleep this thread.
		mem.key = std::tolower(_getch());
		// if game is not paused
		if ( !mem.pause.load() ) {
			// switch player keypress
			switch ( mem.key.load() ) {
			case __BLANK_KEY:break; // player has not pressed a key since last key was processed
			case 'q': // player pressed the exit game key
				mem.kill.store(true);
				std::cout << std::endl << std::endl;
				sys::msg(sys::log, "Kill request received. Shutting down.");
				return 1;
			case 'p': // player pressed the pause game key
				mem.pause.store(true);
				std::this_thread::sleep_for(__MS_CLOCK_SYNC); // wait until the consoleDisplay function is 100% paused
				sys::cls();
				sys::setCursorPos(5, 3);
				std::cout << termcolor::cyan << mem.pause_msg << termcolor::reset;
				break;
			default: // player pressed a different key, process it
				std::scoped_lock<std::mutex> lock(mutx); // lock the mutex
				switch ( game.actionPlayer(mem.key.load()) ) {
				case 0:
					mem.kill.store(true);
					return -1;
				case 2:
					mem.kill.store(true);
					return 0;
				default:break;
				}
				mem.reset_key();
				break;
			}
		} // else check if player wants to unpause
		else if ( mem.key.load() == 'p' ) { mem.pause.store(false); sys::cls(); }
		// else do nothing
	}
	return 2;
}
/**
 * npc(Gamespace&)
 * Processes non-player turns.
 * Loops until the shared memory's kill flag is true
 *
 * @param game	- Reference to the associated gamespace, passed with std::ref()
 */
void npc(Gamespace& game)
{
	// loop until shared memory's kill flag is true
	while ( !mem.kill.load() ) {
		std::this_thread::sleep_for(__MS_CLOCK_SYNC * __NPC_CYCLE_WAIT_MULT);
		if ( !mem.pause.load() ) {
			std::scoped_lock<std::mutex> lock(mutx); // lock the mutex
			game.actionHostile();
		}
		else std::this_thread::sleep_for(std::chrono::seconds(1) - (__MS_CLOCK_SYNC * __NPC_CYCLE_WAIT_MULT));
	}
}
/**
 * consoleDisplay(Gamespace&)
 * Displays the gamespace to the console every half clock cycle
 * Loops until the shared memory's kill flag is true
 *
 * @param game	- Reference to the associated gamespace, passed with std::ref()
 */
void consoleDisplay(Gamespace& game)
{
	typedef std::chrono::high_resolution_clock T;
	// create a frame buffer with the given gamespace ref
	FrameBuffer_Gamespace gameBuffer(game, Coord(3, 3));
	gameBuffer.initConsole(); // set the console window's size & position
	for ( T::time_point tLastRegenCycle{ T::now() }; !mem.kill.load(); ) {
		if ( !mem.pause.load() ) {
			std::this_thread::sleep_for(__MS_CLOCK_SYNC / 2);
			// flush the cout buffer
			std::cout.flush();
			// lock the mutex
			std::scoped_lock<std::mutex> lock(mutx);
			// display the gamespace
			gameBuffer.display();
			if ( (T::now() - tLastRegenCycle) >= std::chrono::seconds(1) ) {
				game.apply_passive();
				tLastRegenCycle = T::now();
			}
		}
		else { // the game is paused
			// set the frame-buffer's initialized flag to false, causing the frame to be reinitialized on unpause
			gameBuffer._initialized = false;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
}

/**
 * game(int, char*[])
 * Controls all threaded game functions. When this function returns, the game is over.
 *
 * @param argc	- From main()
 * @param argv	- From main()
 * @returns int	- ( -1 = Player lost ) ( 0 = Player won ) ( 1 = Player exited ) ( 2 = Undefined termination )
 */
inline int game(GLOBAL& settings)
{
	GameRules rules; // define ruleset
	//rules.trap_damage_is_percentage = false;

	Gamespace thisGame(settings, rules); // create the gamespace
	// start a thread to process player actions
	std::future<int> winState{ std::async(std::launch::async, &play, std::ref(thisGame), std::ref(settings)) };
	std::thread display(&consoleDisplay, std::ref(thisGame)); // start a thread to process display functions
	std::thread enemy(&npc, std::ref(thisGame));			  // start a thread to process enemy actions

	display.join();
	enemy.join();

	if ( !mem.kill.load() )
		return 2;
	return winState.get(); // return the win/lose state to main
}

/**
 * process_game_over(int)
 * This function is used to receive the return code from the game() function, and print a message to the screen indicating the winstate.
 *
 * @param winStateCode	- Received from game()
 */
void process_game_over(int winStateCode)
{
	sys::cls();
	   // switch the win/lose state after game is over
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
	case 2: // undefined shutdown
		std::cout << termcolor::red << "The game did not shut down correctly, exiting...." << termcolor::reset << std::endl;
		break;
	default:break; // undefined return val
	}
	sys::sleep(1000);
	sys::pause("\n\tPress any key to exit....");
	return;
}
