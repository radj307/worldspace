/**
 * Project "worldspace"
 * Wrapper for a matrix of tiles that can be used for simple games. Contains built-in console output functions, but can be used as a backend for graphical applications.
 * ~ Evolved into a basic real-time game application utilizing multi-threading.
 * by radj307
 *
 *	[Task List]
		Add a "Killed by <name>" line to the game over screen
		Add a check when an NPC is pursuing its target to re-apply aggression if the target is still visible.
		Use the get() function to output localized flares rather than full-screen ones.
 */
#include "game_threads.hpp"
#include "opt.h"

inline bool prompt_restart(Coord textPos = Coord(5, 6));
inline std::vector<std::string> interpret(int argc, char* argv[]);

/**
 * main(const int, char*[])
 *
 * @param argc	- Argument count
 * @param argv	- Argument array
 */
int main(const int argc, char* argv[])
{
	try {
		// Keep starting the game until the player doesn't press restart
		do if ( !game::start(interpret(argc, argv)) ) break; while ( prompt_restart() );
		// Return a success code
		return 0;
	} catch ( std::exception& ex ) {
		// Fill the screen buffer with spaces
		sys::cls();
		// Print the exception message
		msg(sys::error, "The game crashed because of an exception: " + std::string(ex.what()), "Press any key to exit....");
		// Return an error code
		return -1;
	}
}

/**
 * prompt_restart(Coord)
 * Prompts the user to press a key and returns true if the key was 'r', false if the key was 'q'
 *
 * @param textPos	- A location in the screen buffer where prompt message will be displayed.
 * @returns bool	- ( true = Restart the game ) ( false = Quit the game )
 */
inline bool prompt_restart(const Coord textPos)
{
	using namespace std::chrono_literals;
	typedef std::chrono::steady_clock T;		// typedef for chrono clock
	const auto TIMEOUT{ std::chrono::duration_cast<std::chrono::nanoseconds>(6s) };	// Maximum time to wait before returning false

	sys::cursorPos(textPos);	// move to target line, print restart key
	std::cout << "Press <" << termcolor::green << 'r' << termcolor::reset << "> to restart.";

	sys::cursorPos(textPos._x, textPos._y + 1);	// move to next line, print quit key
	std::cout << "Press <" << termcolor::red << 'q' << termcolor::reset << "> to quit.";

	// loop until timeout, or valid key press
	for ( const auto t{ T::now() }; T::now() - t <= TIMEOUT; ) {
		if ( _kbhit() ) { // If a key is pressed, process it
			switch ( std::tolower(_getch()) ) {
			case 'r': // r was pressed, restart the game
				return true;
			case 'q': // q was pressed, quit the game
				sys::cursorPos(textPos); // clear text
				std::cout << "                            " << std::endl << "                                 " << std::endl << "                             " << std::endl << "                              " << std::endl;
				return false;
			default:break;
			}
		} // else wait for timeout
		sys::cursorPos(textPos._x + 2, textPos._y + 3);
		std::cout << std::chrono::duration_cast<std::chrono::seconds>(TIMEOUT - (T::now() - t)).count() << "s remaining..." << std::endl;
	}
	sys::cursorPos(textPos); // clear text
	std::cout << "                            " << std::endl << "                                 " << std::endl << "                             " << std::endl << "                              " << std::endl;
	return false;
}

inline std::vector<std::string> interpret(const int argc, char* argv[])
{
	opt::list args(argc, argv, "ini:");
	std::vector<std::string> ini_filelist{};
	for ( auto& it : args._commands )
		if ( it._hasArg && it.checkName("ini") )
			ini_filelist.push_back(it._arg);

#ifdef _DEBUG
	if ( ini_filelist.empty() )
		ini_filelist = { "config.ini", "actor_templates.ini" };
#endif
	
	return ini_filelist;
}