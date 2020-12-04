/**
 * Project "worldspace"
 * Wrapper for a matrix of tiles that can be used for simple games. Contains built-in console output functions, but can be used as a backend for graphical applications.
 * ~ Evolved into a basic real-time game application utilizing multi-threading.
 * by radj307
 * 
 * 
 * 
 * 
 *	[Task List]
		modify the player stat bar to use a Frame rather than direct console output
		notification area for general notifications, that are removed after x seconds
		implement items
 */
#include "game_threads.h"
#include "opt.h"

inline bool prompt_restart(Coord textPos = Coord(5, 6));
inline GLOBAL interpret(int argc, char* argv[]);

/**
 * main(const int, char*[])
 * 
 * @param argc	- Argument count
 * @param argv	- Argument array
 */
int main(const int argc, char* argv[])
{
	// Interpret commandline options
	GLOBAL settings;
	try { settings = { interpret(argc, argv) }; }
	catch( std::exception & ex ) {
		msg(sys::error, "\"" + std::string(ex.what()) + "\" was thrown while interpreting command-line arguments.", "Press any key to exit....");
		return -1;
	}
	do { // Start the game, loop until player declines to restart, or player pressed the quit button
		try { // Check the return code from game::start
			if ( game::start(settings) ) break; // Break without prompting for a restart
		} catch ( std::exception& ex ) {
			WinAPI::cls();
			msg(sys::error, "The game crashed with exception: \"" + std::string(ex.what()) + "\"", "PRESS ANY KEY TO EXIT....");
			return -1;
		}
	} while ( prompt_restart() );
	return 0;
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
	typedef std::chrono::steady_clock T;		// typedef for chrono clock
	const std::chrono::seconds _TIMEOUT{ 6 };	// Maximum time to wait before returning false

	WinAPI::setCursorPos(textPos);	// move to target line, print restart key
	std::cout << "Press <" << termcolor::green << 'r' << termcolor::reset << "> to restart.";

	WinAPI::setCursorPos(textPos._x, textPos._y + 1);	// move to next line, print quit key
	std::cout << "Press <" << termcolor::red << 'q' << termcolor::reset << "> to quit.";

	// loop until timeout, or valid key press
	for ( auto t{ T::now() }; T::now() - t <= _TIMEOUT; t = T::now() ) {
		if ( _kbhit() ) { // If a key is pressed, process it
			switch ( std::tolower(_getch()) ) {
			case 'r': // r was pressed, restart the game
				return true;
			case 'q': // q was pressed, quit the game
				return false;
			default:break;
			}
		} // else wait for timeout
	}
	return false;
}

/**
 * GLOBAL interpret(int, const char*[])
 * Returns a GLOBALS instance with parsed command line arguments
 *
 * @param argc	- From main()
 * @param argv	- From main()
 * @returns GLOBAL
 */
inline GLOBAL interpret(const int argc, char* argv[])
{
	GLOBAL glob;

	opt::list args(argc, argv, "world:,file:,player:");
	for (auto& _command : args._commands) {
		// "-world <arg>"
		if (_command.checkName("world") && _command._hasArg ) {
			// "-world showalltiles"
			if (_command.checkArg("showalltiles") ) {
				glob._override_known_tiles = true;
			}
			// "-world size=XX:XX"
			else if (_command._arg.size() >= 8 && _command._arg.substr(0, _command._arg.find('=')) == "size" ) {
				if (_command._arg.find(':') != std::string::npos ) {
					auto str{ (_command._arg.substr(_command._arg.find('=') + 1, _command._arg.size())) };
					try {
						const auto index_of_colon{ str.find(':') };
						glob._cellSize = Coord(std::stoi(str.substr(0, index_of_colon)), std::stoi(str.substr(index_of_colon + 1)));
					} catch ( std::exception &ex ) {
						msg(sys::warning, "Argument for world size threw exception: " + std::string(ex.what()));
					}
				}
			}
		}
		// "-file <filename>"
		else if (_command.checkName("file") && _command._hasArg ) {
			glob._import_filename = _command._arg;
		}
		// "-player <arg>"
		else if (_command.checkName("player") ) {
			// Check complex arguments
			const auto index{_command._arg.find('=') };
			if ( index != std::string::npos ) {
				auto arg{_command._arg.substr(0, index) };
				try {
					if ( arg == "health=" )
						glob._player_health = std::stoi(_command._arg.substr(index + 1));
					else if ( arg == "stamina=" )
						glob._player_stamina = std::stoi(_command._arg.substr(index + 1));
					else if ( arg == "damage=" )
						glob._player_damage = std::stoi(_command._arg.substr(index + 1));
				} catch ( std::exception & ex ) {
					sys::msg(sys::warning, "Argument for player statistic \"" + arg + "\" threw exception: \"" + std::string(ex.what()) + "\"");
				}
			}
			// Check simple arguments
			else {
				if (_command.checkArg("godmode") )
					glob._player_godmode = true;
			}
		}
	}

	return glob;
}
