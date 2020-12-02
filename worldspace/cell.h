/**
 * world.h
 * Represents the game world.
 * Contains the Cell class, and the Tile struct.
 * by radj307
 */
#pragma once
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>
#include <Windows.h>

#include "Coord.h"
#include "file.h"
#include "WinAPI.h"
#include "xRand.h"

/**
 * struct Tile : public Coord
 * Represents a single position in the matrix of a cell
 */
struct Tile : Coord {
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

	display _display;								// the display character
	WinAPI::color _myColor{ WinAPI::color::white };	// The color of the display character
	bool _isKnown;									// true if this tile is known to the player
	bool _canMove;									// true if this tile allows actors to move to it
	bool _isTrap;									// true if this tile is a trap, and will damage actors when stepped on.

	/** CONSTRUCTOR **
	 * Tile(Tile::display, int, int, bool)  
	 * Construct a tile with the default color. (white)
	 * 
	 * @param as				- This tile's type (display character)
	 * @param xPos				- The X (horizontal) index of this tile in relation to the matrix
	 * @param yPos				- The Y (vertical) index of this tile in relation to the matrix
	 * @param makeWallsVisible	- When true, all wall tiles are always visible to the player
	 * @param isKnownOverride	- When true, this tile is visible to the player.
	 */
	Tile(const display as, const int xPos, const int yPos, const bool makeWallsVisible, const bool isKnownOverride) : Coord(xPos, yPos), _display(as), _isKnown(isKnownOverride), _canMove(true), _isTrap(false)
	{
		// check if tile attributes are correct
		switch ( _display ) {
		case display::none:
			_canMove = false;
			break;
		case display::wall:
			if ( makeWallsVisible )
				_isKnown = true;
			_canMove = false;
			break;
		case display::hole:
			_isTrap = true;
			break;
		default:break;
		}
	}

	/** CONSTRUCTOR **
	 * Tile(Tile::display, int, int, bool)
	 * Construct a tile with a defined color.
	 *
	 * @param as				- This tile's type (display character)
	 * @param color				- This tile's color when inserted into a stream.
	 * @param xPos				- The X (horizontal) index of this tile in relation to the matrix
	 * @param yPos				- The Y (vertical) index of this tile in relation to the matrix
	 * @param makeWallsVisible	- walls are always visible
	 * @param isKnownOverride	- When true, this tile is visible to the player.
	 */
	Tile(const display as, const WinAPI::color color, const int xPos, const int yPos, const bool makeWallsVisible, const bool isKnownOverride) : Coord(xPos, yPos), _display(as), _myColor(color), _isKnown(isKnownOverride), _canMove(true), _isTrap(false)
	{
		// check if tile attributes are correct
		switch ( _display ) {
		case display::none:
			_canMove = false;
			break;
		case display::wall:
			if ( makeWallsVisible )
				_isKnown = true;
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

	// comparison operators
	bool operator==(Tile &o) const
	{
		if ( _display == o._display )
			return true;
		return false;
	}
	bool operator!=(Tile &o) const
	{
		if ( _display != o._display )
			return true;
		return false;
	}
	bool operator==(Tile::display o) const
	{
		if ( _display == o )
			return true;
		return false;
	}
	bool operator!=(Tile::display o) const
	{
		if ( _display != o )
			return true;
		return false;
	}

	// Stream insertion operator
	friend std::ostream& operator<<(std::ostream& os, const Tile& t)
	{
		if ( t._myColor != WinAPI::color::white ) // if this tile has a color, set it
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<int>(t._myColor));
		if ( t._isKnown ) // if this tile is known, insert its char
			os << static_cast<char>(t._display) << ' ';
		else // if this tile is not known, insert a blank
			os << ' ' << ' ';
		// reset color
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0);
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
 * @param makeWallsVisible		- When true, wall tiles will always be visible.
 * @param override_known_tiles	- When true, all tiles will be visible to the player from the start.
 * @returns vector<vector<Tile>>
 */
inline std::vector<std::vector<Tile>> importMatrix(const std::string& filename, const bool makeWallsVisible, const bool override_known_tiles)
{
	std::vector<std::vector<Tile>> matrix{};

	if ( file::exists(filename) ) {
		auto content{ file::readToStream(filename) };

		content.seekg(0, std::ios::end);				// send sstream to the end
		const auto content_size{ static_cast<int>(content.tellg()) };  // find the size of received stringstream
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
					auto isValid{ false };
					for ( auto it = __VALID_TILES.begin(); it != __VALID_TILES.end(); it++ ) {
						if ( line.at(x) == static_cast<char>(*it) )
							isValid = true;
					}
					if ( isValid )	row.emplace_back(static_cast<Tile::display>(line.at(x)), x, y, makeWallsVisible, override_known_tiles);
					else			row.emplace_back(Tile::display::none, x, y, makeWallsVisible, override_known_tiles);
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
	 * generate(bool, bool)
	 * Generates the tile matrix using RNG
	 * 
	 * @param makeWallsVisible		- When true, wall tiles will always be visible.
	 * @param override_known_tiles	- When true, all tiles will be visible to the player from the start.
	 */
	void generate(bool makeWallsVisible, bool override_known_tiles)
	{
		if ( _max._y > 5 && _max._x > 5 ) {
			tRand rng;
			for (auto y = 0; y < _max._y; y++ ) {
				std::vector<Tile> _row;
				for (auto x = 0; x < _max._x; x++ ) {
					// make walls on all edges
					if ( x == 0 || x == _max._x - 1 || (y == 0 || y == _max._y - 1) )
						_row.emplace_back(Tile::display::wall, x, y, makeWallsVisible, override_known_tiles);
					else { // not an edge
						const auto rand = rng.get(100u, 0u);
						if ( rand < 7 ) // 7:100 chance of a wall tile that isn't on an edge
							_row.emplace_back(Tile::display::wall, x, y, makeWallsVisible, override_known_tiles);
						else if ( rand > 7 && rand < 9 )
							_row.emplace_back(Tile::display::hole, x, y, makeWallsVisible, override_known_tiles);
						else
							_row.emplace_back(Tile::display::empty, x, y, makeWallsVisible, override_known_tiles);
					}
				}
				_matrix.push_back(_row);
			}
		}
	}

public:
	// The Cell's Vertical & Horizontal size as a Coord
	const Coord _max;
	// Functor that checks if a given tile is within the cell boundaries
	checkBounds isValidPos; // function syntax is used to emulate a member function

	/** CONSTRUCTOR **
	 * Cell(Coord, bool)
	 * Generate a new cell with the given size parameters
	 * 
	 * @param cellSize				- The size of the cell
	 * @param makeWallsVisible		- walls are always visible
	 * @param override_known_tiles	- When true, all tiles will be visible to the player from the start.
	 */
	explicit Cell(const Coord& cellSize, const bool makeWallsVisible = true, const bool override_known_tiles = false) : _max(cellSize._x - 1, cellSize._y - 1), isValidPos(_max)
	{
		generate(makeWallsVisible, override_known_tiles);
	}

	/** CONSTRUCTOR **
	 * Cell(string, bool)
	 * Load a cell from a specified file
	 * 
	 * @param filename				- Target file to load, must be formatted correctly or '?' tiles will appear.
	 * @param makeWallsVisible		- walls always visible
	 * @param override_known_tiles	- When true, all tiles will be visible to the player from the start.
	 */
	explicit Cell(const std::string& filename, const bool makeWallsVisible = true, const bool override_known_tiles = false) : 
		_matrix(importMatrix(filename, makeWallsVisible, override_known_tiles)), 
		_max(_matrix.at(0).size() - 1, _matrix.size() - 1),
		isValidPos(_max) {}

	/**
	 * display()
	 * Print the cell to the console as it is known to the player.
	 */
	void display()
	{
		std::stringstream buf;
		for ( auto y = _matrix.begin(); y != _matrix.end(); y++ ) {
			for ( auto x = y->begin(); x != y->end(); x++ ) {
				buf << *x;
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
	void display(const Coord& pos, const int diameter)
	{
		std::stringstream buf;
		// iterate vertical
		for ( int y = static_cast<int>(pos._y - diameter); y < static_cast<int>(pos._y + diameter); y++ ) { 
			// counter for number of chars added
			int doNewline{ 0 };
			// iterate horizontal
			for ( int x = static_cast<int>(pos._x - diameter); x < static_cast<int>(pos._x + diameter); x++ ) { 
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
	char getDisplayChar(const Coord& pos)
	{
		if ( isValidPos(pos) )
			return static_cast<char>(get(pos)._display);
		return NULL;
	}

	/**
	 * modVis(bool)  
	 * Modifies the visibility of all tiles in the cell.
	 * 
	 * @param to		- ( true = visible ) ( false = invisible )
	 */
	void modVis(bool to)
	{
		for ( auto y = _matrix.begin(); y != _matrix.end(); y++ )
			for ( auto x = y->begin(); x != y->end(); x++ )
				x->_isKnown = to;
	}

	/**
	 * modVis(bool, Coord, const int)  
	 * Modifies the visibility of an area around a given center-point in the cell.
	 * 
	 * @param to		- ( true = visible ) ( false = invisible )
	 * @param pos		- The center-point
	 * @param diameter	- The distance away from the center-point that will also be discovered.
	 */
	void modVis(bool to, const Coord& pos, const int diameter = 3)
	{
		for ( int y = pos._y - diameter; y <= pos._y + diameter; y++ )
			for ( int x = pos._x - diameter; x <= pos._x + diameter; x++ )
				if ( isValidPos(x, y) )
					_matrix.at(y).at(x)._isKnown = to;
	}

	/**
	 * modVis(bool, Coord, Coord)
	 * Modifies the visibility of a specified area in the cell.
	 *
	 * @param to		- ( true = visible ) ( false = invisible )
	 * @param minPos	- The top-left corner of the target area
	 * @param maxPos	- The bottom-right corner of the target area
	 */
	void modVis(bool to, const Coord& minPos, const Coord& maxPos)
	{
		for ( int y = minPos._y; y <= maxPos._y; y++ )
			for ( int x = minPos._x; x <= maxPos._x; x++ )
				if ( isValidPos(x, y) )
					_matrix.at(y).at(x)._isKnown = to;
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
			for ( auto x = (*y).begin(); x != (*y).end(); x++ )
				buf << *x;
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
	Tile &get(const Coord& pos)
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
	Tile &get(int x, int y)
	{
		if ( isValidPos(x, y) )
			return _matrix.at(y).at(x);
		return __TILE_ERROR;
	}

	/**
	 * exportCell(string)
	 * Exports this cell to a file.
	 *
	 * @param filename	- The name of the output file
	 * @param saveAs	- Whether to overwrite or append to file if it already exists.
	 * @returns bool	- true when successful, false when failed.
	 */
	bool exportToFile(std::string filename, file::save_type saveAs = file::save_type::overwrite)
	{
		return file::write(std::move(filename), sstream(), saveAs);
	}
};