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

inline GLOBAL interpret(int argc, char* argv[]);

/**
 * main(const int, char*[])
 * 
 * @param argc	- Argument count
 * @param argv	- Argument array
 */
auto main(const int argc, char* argv[]) -> int
{
	try { // run game
		game_start(interpret(argc, argv));
		// return 0 for successful execution
		return 0;
	} catch ( ... ) {
		msg(sys::error, "An unknown error occurred during game operations.", "PRESS ANY KEY TO EXIT....");
		// return -1 for errors
		return -1;
	}
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
	for ( auto it = args._commands.begin(); it != args._commands.end(); ++it ) {
		// "-world <arg>"
		if ( (*it).checkName("world") && (*it)._hasArg ) {
			// "-world showalltiles"
			if ( (*it).checkArg("showalltiles") ) {
				glob._override_known_tiles = true;
			}
			// "-world size=XX:XX"
			else if ( (*it)._arg.size() >= 8 && (*it)._arg.substr(0, (*it)._arg.find('=')) == "size" ) {
				if ( (*it)._arg.find(':') != std::string::npos ) {
					auto str{ ((*it)._arg.substr((*it)._arg.find('=') + 1, (*it)._arg.size())) };
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
		else if ( (*it).checkName("file") && (*it)._hasArg ) {
			glob._import_filename = (*it)._arg;
		}
		// "-player <arg>"
		else if ( (*it).checkName("player") ) {
			// Check complex arguments
			const auto index{ (*it)._arg.find('=') };
			if ( index != std::string::npos ) {
				auto arg{ (*it)._arg.substr(0, index) };
				try {
					if ( arg == "health=" )
						glob._player_health = std::stoi((*it)._arg.substr(index + 1));
					else if ( arg == "stamina=" )
						glob._player_stamina = std::stoi((*it)._arg.substr(index + 1));
					else if ( arg == "damage=" )
						glob._player_damage = std::stoi((*it)._arg.substr(index + 1));
				} catch ( std::exception & ex ) {
					sys::msg(sys::warning, "Argument for player statistic \"" + arg + "\" threw exception: \"" + std::string(ex.what()) + "\"");
				}
			}
			// Check simple arguments
			else {
				if ( (*it).checkArg("godmode") )
					glob._player_godmode = true;
			}
		}
	}

	return glob;
}
