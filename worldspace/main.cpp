#include "world.h"
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
inline GLOBAL interpret(int argc, const char* argv[])
{
	GLOBAL glob(false, false, 25, 25);

	opt::list args(argc, argv);
	for ( auto it = args.com.begin(); it != args.com.end(); it++ ) {
		if ( (*it).checkName("world") ) {
			if ( (*it).checkOpt("tiles") && (*it).checkParam("unhide") )
				glob._override_known_tiles = true;
			else if ( (*it).checkOpt("size") ) {
				std::string* ptr = &(*it)._param;				// make a local pointer to command param
				int index_of_divider = (*ptr).find(',');		// find comma
				if ( index_of_divider != std::string::npos ) {	// if comma was found
					try {
						glob._cellSize._x = std::stoi(ptr->substr(0, index_of_divider));
						glob._cellSize._y = std::stoi(ptr->substr(index_of_divider + 1, (ptr->size() - 1)));
					}
					catch ( std::exception & const except ) {	// catch stoi exceptions
						sys::msg(sys::warning, "Interpreting world size argument threw exception: " + std::string(except.what()) + " -- Using default: (" + std::to_string(glob._cellSize._x) + "/" + std::to_string(glob._cellSize._y) + ")");
					}
				}												// else do nothing
			}
		}
		if ( (*it).checkName("debug") ) {
			glob._debug_msg = true;
		}
	}

	return glob;
}

int main(int argc, const char* argv[])
{


	return 0;
}