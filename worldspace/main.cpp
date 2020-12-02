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
		add automatic regeneration of stats, such as stamina, over time.
		modify the player stat bar to use a Frame rather than direct console output
		player feedback for events
			dead enemies change the color of the tile they died on for a few frames?
			add a text feedback area to the stat display?
			add another stat display box for text feedback?
		notification area for general notifications, that are removed after x seconds
		IMPROVE LEVEL UPS -- Player stat bar algorithm, some kind of notification. (player color flashes? area around player changes color temporarily? additional Level display at the bottom?)

	[BACKBURNER]
		implement items
 */
#include "game_threads.h"
#include "opt.h"

inline GLOBAL interpret(int argc, char* argv[]);

// commandline to set window size:
// -resolution 800x600
// commandline to load cell from file:
// -file 20x20.txt
int main(const int argc, char* argv[])
{
	// Interpret commandline arguments
	//GLOBAL settings{ interpret(argc, argv) };

	// run game
	game_start(interpret(argc, argv));

	// return 0 for successful execution
	return 0;
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

	opt::list args(argc, argv, "world:,file:");
	for ( auto it = args._commands.begin(); it != args._commands.end(); ++it ) {
		if ( (*it).checkName("world") && (*it)._hasArg ) {
			if ( (*it).checkArg("showalltiles") ) {
				glob._override_known_tiles = true;
			}
			else if ( (*it)._arg.size() >= 8 && (*it)._arg.substr(0, (*it)._arg.find('=')) == "size" ) {
				if ( (*it)._arg.find(':') != std::string::npos ) {
					auto str{ ((*it)._arg.substr((*it)._arg.find('=') + 1, (*it)._arg.size())) };
					try {
						const auto index_of_colon{ str.find(':') };
						glob._cellSize = Coord(std::stoi(str.substr(0, index_of_colon)), std::stoi(str.substr(index_of_colon + 1)));
					} catch ( std::exception &ex ) {
						sys::msg(sys::warning, "Argument for world size threw exception: " + std::string(ex.what()));
					}
				}
			}
		}
		else if ( (*it).checkName("file") && (*it)._hasArg ) {
			glob._import_filename = (*it)._arg;
		}
	}

	return glob;
}
