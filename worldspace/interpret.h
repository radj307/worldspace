#pragma once
#include "opt.h"
#include "sys.h"
#include "settings.h"
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
	GLOBAL glob;

	opt::list args(argc, argv, "world:,resolution:,file:");
	for ( auto it = args._commands.begin(); it != args._commands.end(); it++ ) {
		if ( (*it).checkName("world") && (*it)._hasArg ) {
			if ( (*it).checkArg("showalltiles") ) {
				glob._override_known_tiles = true;
			}
			else if ( ((*it)._arg.size() >= 8) && ((*it)._arg.substr(0, (*it)._arg.find('=')) == "size") ) {
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
		else if ( (*it).checkName("resolution") && (*it)._hasArg ) {
			std::string str{ (*it)._arg };
			size_t index_of_x = str.find('x');
			if ( index_of_x != std::string::npos ) {
				try {
					glob._resolution = Coord(std::stoi(str.substr(0, index_of_x)), std::stoi(str.substr(index_of_x + 1)));
				} catch ( std::exception & ex ) {
					sys::msg(sys::warning, "Argument for screen resolution threw exception: " + std::string(ex.what()));
				}
			} // else continue
		}
		else if ( (*it).checkName("file") && (*it)._hasArg ) {
			glob._import_filename = (*it)._arg;
		}
	}

	return glob;
}