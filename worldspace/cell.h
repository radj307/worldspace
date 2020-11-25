/**
 * world.h
 * Represents the game world.
 * Contains the Cell class, and the Tile struct.
 * by radj307
 */
#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include <Windows.h>
#include "xRand.h"
#include "Coord.h"
#include "file.h"

/**
 * struct Tile : public Coord
 * Represents a single position in the matrix of a cell
 */
struct Tile : public Coord {
	// Windows console text colors
	enum class color {
		white = 0x0001 | 0x0002 | 0x0004,
		grey = 0,
		yellow = 0x0002 | 0x0004,
		red = 0x0004,
		magenta = 0x0001 | 0x0004,
		blue = 0x0001,
		cyan = 0x0001 | 0x0002,
		green = 0x0002,
	}; color _myColor{ color::white };
	/**
	 * enum class display
	 * Defines valid tile types/display characters.
	 * Remember to add new entries to vector __VALID_TILES below
	 */
	enum class display {
		none = '?',
		empty = '_',
		wall = '#',
		hole = 'O',
	};
	// the display character
	display _display;
	// if this tile is known to the player or not
	bool _isKnown;
	bool _canMove;
	bool _isTrap;

	/** CONSTRUCTOR **
	 * Tile(Tile::display, int, int, bool)  
	 * 
	 * @param as				- This tile's type (display character)
	 * @param xPos				- The X (horizontal) index of this tile in relation to the matrix
	 * @param yPos				- The Y (vertical) index of this tile in relation to the matrix
	 * @param isKnownOverride	- When true, this tile is visible to the player.
	 */
	Tile(display as, int xPos, int yPos, bool isKnownOverride = false) : Coord(xPos, yPos), _display(as), _isKnown(isKnownOverride), _canMove(true), _isTrap(false)
	{
		// check if tile attributes are correct
		switch ( _display ) {
		case display::none:
			_canMove = false;
			break;
		case display::wall:
			_canMove = false;
			break;
		case display::hole:
			_isTrap = true;
			break;
		default:break;
		}
	}
	/** CONSTRUCTOR **
	 * Tile()
	 * Instantiate a blank tile with coordinate of (-1,-1) and no type.
	 */
	Tile() : Coord(-1, -1), _display(display::none), _isKnown(true), _canMove(false), _isTrap(false) {}

	bool operator==(Tile &o)
	{
		if ( _display == o._display )
			return true;
		return false;
	}
	bool operator!=(Tile &o)
	{
		if ( _display != o._display )
			return true;
		return false;
	}
	bool operator==(Tile::display o)
	{
		if ( _display == o )
			return true;
		return false;
	}
	bool operator!=(Tile::display o)
	{
		if ( _display != o )
			return true;
		return false;
	}

	// Stream insertion operator
	friend inline std::ostream& operator<<(std::ostream& os, const Tile& t)
	{
		if ( t._myColor != color::white )
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<int>(t._myColor)); // set text color to 
		if ( t._isKnown )
			os << char(t._display) << ' ';
		else
			os << ' ' << ' ';
		return os;
	}
};
static const std::vector<Tile::display> __VALID_TILES{ Tile::display::none, Tile::display::empty, Tile::display::wall, Tile::display::hole };
static Tile __TILE_ERROR;

/**
 * importMatrix(string, bool)
 * Imports a tile matrix from the specified file
 * 
 * @param filename				- The target file to load
 * @param override_known_tiles	- When true, all tiles will be visible to the player from the start.
 * @returns vector<vector<Tile>>
 */
std::vector<std::vector<Tile>> importMatrix(std::string filename, bool override_known_tiles = false)
{
	std::vector<std::vector<Tile>> matrix{};

	if ( file::exists(filename) ) {
		std::stringstream content{ file::readToStream(filename) };

		content.seekg(0, std::ios::end);				// send sstream to the end
		int content_size = static_cast<int>(content.tellg());  // find the size of received stringstream
		content.seekg(0, std::ios::beg);				// send sstream back to the beginning

		// check if content is not empty
		if ( content_size > 0 ) {
			std::string line{};
			// iterate through file content by line
			for ( unsigned int y = 0; std::getline(content, line); y++ ) {
				std::vector<Tile> row;
				// remove whitespace
				line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
				// iterate through line
				for ( unsigned int x = 0; x < line.size(); x++ ) {
					bool isValid{ false };
					for ( auto it = __VALID_TILES.begin(); it != __VALID_TILES.end(); it++ ) {
						if ( line.at(x) == char(*it) )
							isValid = true;
					}
					if ( isValid )	row.push_back(Tile(static_cast<Tile::display>(line.at(x)), x, y, override_known_tiles));
					else			row.push_back(Tile(Tile::display::none, x, y, override_known_tiles));
				}
				matrix.push_back(row);
				line.clear();
			}
		}
	}
	return matrix;
}

class Cell {
	// a matrix of tiles
	std::vector<std::vector<Tile>> _matrix;

protected:

	/**
	 * generate(bool)
	 * Generates the matrix
	 * 
	 * @param override_known_tiles	- When true, all tiles will be visible to the player from the start.
	 */
	inline void generate(bool override_known_tiles)
	{
		if ( _sizeV > 5 && _sizeH > 5 ) {
			tRand rng;

			for ( int y = 0; y < _sizeV; y++ ) {
				std::vector<Tile> _row;
				for ( int x = 0; x < _sizeH; x++ ) {
					// make walls on all edges
					if ( (x == 0 || x == (_sizeH - 1)) || (y == 0 || y == (_sizeV - 1)) )
						_row.push_back(Tile(Tile::display::wall, x, y, override_known_tiles));
					else { // not an edge
						unsigned int rand = rng.get(100u, 0u);
						if ( rand < 7 ) // 7:100 chance of a wall tile that isn't on an edge
							_row.push_back(Tile(Tile::display::wall, x, y, override_known_tiles));
						else if ( rand > 7 && rand < 9 )
							_row.push_back(Tile(Tile::display::hole, x, y, override_known_tiles));
						else
							_row.push_back(Tile(Tile::display::empty, x, y, override_known_tiles));
					}
				}
				_matrix.push_back(_row);
			}
		}
	}

public:
	// The Cell's Vertical & Horizontal size
	const int _sizeV, _sizeH;
	// Functor that checks if a given tile is within the cell boundaries
	checkBounds isValidPos;

	/** CONSTRUCTOR **
	 * Cell(Coord, bool)
	 * Generate a new cell with the given size parameters
	 * 
	 * @param cellSize				- The size of the cell
	 * @param override_known_tiles	- When true, all tiles will be visible to the player from the start.
	 */
	Cell(Coord cellSize, bool override_known_tiles = false) : _sizeH((signed)cellSize._x), _sizeV((signed)cellSize._y), isValidPos(Coord(_sizeH, _sizeV))
	{
		generate(override_known_tiles);
	}

	/** CONSTRUCTOR **
	 * Cell(string, bool)
	 * Load a cell from a specified file
	 * 
	 * @param filename				- Target file to load, must be formatted correctly or '?' tiles will appear.
	 * @param override_known_tiles	- When true, all tiles will be visible to the player from the start.
	 */
	Cell(std::string filename, bool override_known_tiles = false) : _matrix(importMatrix(filename, override_known_tiles)), _sizeH((file::exists(filename)) ? (_matrix.at(0).size()) : (0)), _sizeV((file::exists(filename)) ? (_matrix.size()) : (0)), isValidPos(Coord(_sizeH, _sizeV)) {}

	/**
	 * display()
	 * Print the cell to the console as it is known to the player.
	 */
	inline void display()
	{
		std::stringstream buf;
		for ( auto y = _matrix.begin(); y != _matrix.end(); y++ ) {
			for ( auto x = y->begin(); x != y->end(); x++ ) {
				buf << (*x);
			}
			buf << std::endl;
		}
		std::cout << buf.rdbuf();
	}

	/**
	 * display(Coord, const int)
	 * Print a section of the cell to the console.
	 * 
	 * @param pos
	 * @param diameter
	 */
	inline void display(Coord pos, const int diameter)
	{
		std::stringstream buf;
		// iterate vertical
		for ( int y = (signed)(pos._y - diameter); y < (signed)(pos._y + diameter); y++ ) { 
			// counter for number of chars added
			int doNewline{ 0 };
			// iterate horizontal
			for ( int x = (signed)(pos._x - diameter); x < (signed)(pos._x + diameter); x++ ) { 
				// check if this pos exists
				if ( isValidPos(x, y) ) { 
					buf << _matrix.at(y).at(x);
					doNewline++;
				}
			}
			// check if a newline should be inserted
			if ( doNewline )
				buf << std::endl;
		}
		std::cout << buf.rdbuf();
	}

	/**
	 * display(Coord)
	 * Returns the character
	 *
	 * @param pos	- Target position
	 */
	inline char getDisplayChar(Coord pos)
	{
		if ( isValidPos(pos) )
			return char(get(pos)._display);
		return{ NULL };
	}

	/**
	 * modVis(bool)  
	 * Modifies the visibility of all tiles in the cell.
	 * 
	 * @param to		- ( true = visible ) ( false = invisible )
	 */
	inline void modVis(bool to)
	{
		for ( auto y = _matrix.begin(); y != _matrix.end(); y++ ) {
			for ( auto x = y->begin(); x != y->end(); x++ ) {
				x->_isKnown = to;
			}
		}
	}

	/**
	 * modVis(bool, Coord, const int)  
	 * Modifies the visibility of an area around a given center-point in the cell.
	 * 
	 * @param to		- ( true = visible ) ( false = invisible )
	 * @param pos		- The center-point
	 * @param diameter	- The distance away from the center-point that will also be discovered.
	 */
	inline void modVis(bool to, Coord pos, const int diameter = 3)
	{
		for ( int y = (pos._y - diameter); y <= (pos._y + diameter); y++ ) {
			for ( int x = (pos._x - diameter); x <= (pos._x + diameter); x++ ) {
				if ( isValidPos(x, y) )
					_matrix.at(y).at(x)._isKnown = to;
			}
		}
	}

	/**
	 * modVis(bool, Coord, Coord)
	 * Modifies the visibility of a specified area in the cell.
	 *
	 * @param to		- ( true = visible ) ( false = invisible )
	 * @param minPos	- The top-left corner of the target area
	 * @param maxPos	- The bottom-right corner of the target area
	 */
	inline void modVis(bool to, Coord minPos, Coord maxPos)
	{
		for ( int y = minPos._y; y <= maxPos._y; y++ ) {
			for ( int x = minPos._x; x <= maxPos._x; x++ ) {
				if ( isValidPos(x, y) )
					_matrix.at(y).at(x)._isKnown = to;
			}
		}
	}

	/**
	 * sstream()  
	 * Returns the entire matrix as a stringstream for file export/import
	 * 
	 * @returns stringstream
	 */
	std::stringstream sstream()
	{
		std::stringstream buf;
		for ( auto y = _matrix.begin(); y != _matrix.end(); y++ ) {
			for ( auto x = (*y).begin(); x != (*y).end(); x++ ) {
				buf << *x;
			}
			buf << std::endl;
		}
		return buf;
	}

	/**
	 * get(Coord, const bool)  
	 * Returns a reference to the target tile.
	 * 
	 * @param pos			- The target tile
	 */
	inline Tile &get(Coord pos)
	{
		if ( isValidPos(pos) )
			return _matrix.at(pos._y).at(pos._x);
		return __TILE_ERROR;
	}

	/**
	 * get(Coord, const bool)
	 * Returns a reference to the target tile.
	 *
	 * @param x				- The target tile's x index
	 * @param y				- The target tile's y index
	 */
	inline Tile &get(int x, int y)
	{
		if ( isValidPos(x, y) )
			return _matrix.at(y).at(x);
		return __TILE_ERROR;
	}
};

/**
 * exportCell(string, Cell&)
 * Exports a given cell to a file
 * 
 * @param filename	- The name of the output file
 * @param cell		- A ref to the target cell
 * @returns bool	- true when successful, false when failed.
 */
bool exportCell(std::string filename, Cell &cell)
{
	return file::write(filename, cell.sstream(), file::save_type::overwrite);
}