/**
 * Project "worldspace"
 * Wrapper for a matrix of tiles that can be used for simple games. Contains built-in console output functions, but can be used as a backend for graphical applications.
 * by radj307
 * 
 *	  [File Inheritance Structure]
 *			   <Coord.h>
 *			   v	  v
 *		 <actor.h>   <cell.h>
 *			   v      v
 *			   <game.h>
 */
#include "game.h"
#include "sys.h"
#include "opt.h"

/**
 * WorldAttributes
 * Contains all settings used by world generation process.
 */
struct WorldAttributes {
	Coord _cellSize;
	bool _override_known_tiles;
	WorldAttributes(bool override_hidden, unsigned int horizontalSize, unsigned int verticalSize) : _override_known_tiles(override_hidden), _cellSize(Coord(horizontalSize, verticalSize)) {}
};

/**
 * GLOBAL <- WorldAttributes
 * Contains all global settings, and derived world attribute settings.
 */
struct GLOBAL : public WorldAttributes {
	bool _debug_msg;
	GLOBAL(bool debug, bool override_hidden, unsigned int horizontalSize, unsigned int verticalSize) : WorldAttributes(override_hidden, horizontalSize, verticalSize), _debug_msg(debug) {}
};

/**
 * GLOBAL interpret(int, const char*[])
 * Returns a GLOBALS instance with parsed command line arguments
 * 
 * @param argc	- From main()
 * @param argv	- From main()
 * @returns GLOBAL
 */
inline GLOBAL interpret(int argc, char* argv[])
{
	GLOBAL glob(false, false, 25, 25);

	opt::list args(argc, argv, "world:,debug");
	for ( auto it = args._commands.begin(); it != args._commands.end(); it++ ) {
		if ( (*it).checkName("world") && (*it)._hasArg ) {
			if ( (*it).checkArg("showalltiles") ) {
				glob._override_known_tiles = true;
			}
			else if ( ((*it)._arg.size() >= 6) && ((*it)._arg.substr(0, (*it)._arg.find('=')) == "size") ) {
				if ( (*it)._arg.find(':') != std::string::npos ) {
					std::string str{ ((*it)._arg.substr(((*it)._arg.find('=') + 1), (*it)._arg.size())) };
					try {
						size_t index_of_colon{ str.find(':') };
						glob._cellSize = Coord(std::stoi(str.substr(0, index_of_colon)), std::stoi(str.substr(index_of_colon + 1)));
					} catch ( std::exception &ex ) {
						sys::msg(sys::warning, "Argument for world size threw exception: " + std::string(ex.what()));
					}
				}
			}
		}
		if ( (*it).checkName("debug") && !(*it)._hasArg ) {
			glob._override_known_tiles = true;
			glob._debug_msg = true;
		}
	}

	return glob;
}

int main(int argc, char* argv[])
{
	GLOBAL settings{ interpret(argc, argv) };

	Cell c(settings._cellSize, settings._override_known_tiles);
	Gamespace g(c);
	g._world.display();

	return 0;
}