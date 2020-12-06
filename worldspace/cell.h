/**
 * world.h
 * Represents the game world.
 * Contains the Cell class, and the Tile struct.
 * by radj307
 */
#pragma once
//#include <iostream>
#include <sstream>
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
struct Tile final : Coord {
private:
	/**
	 * initTraits(bool)
	 * Initialize this tile's variables based on type.
	 *
	 * @param makeWallsVisible	- When true, all wall tiles will always be known to the player
	 */
	void initTraits(const bool makeWallsVisible)
	{
		// check if tile attributes are correct
		switch ( _display ) {
		case display::none:break;
		case display::wall:
			if ( makeWallsVisible )
				_isKnown = true;
			break;
		case display::hole:
			_canMove = true;
			_isTrap = true;
			break;
		case display::empty:
			_canMove = true;
			_canSpawn = true;
			break;
		}
	}

public:

	/**
	 * enum class display
	 * Defines valid tile types/display characters.
	 * Remember to add new entries to vector __VALID_TILES below
	 */
	enum class display {
		empty = '_',
		wall = '#',
		hole = 'O',
		none = '?',
	};

	display _display;								// the display character
	WinAPI::color _myColor{ WinAPI::color::white };	// The color of the display character
	bool _isKnown;									// true if this tile is known to the player
	bool _canMove;									// true if this tile allows actors to move to it
	bool _isTrap;									// true if this tile is a trap, and will damage actors when stepped on.
	bool _canSpawn;

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
	Tile(const display as, const int xPos, const int yPos, const bool makeWallsVisible, const bool isKnownOverride) : Coord(xPos, yPos), _display(as), _isKnown(isKnownOverride), _canMove(false), _isTrap(false), _canSpawn(false)
	{
		initTraits(makeWallsVisible);
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
	Tile(const display as, const WinAPI::color color, const int xPos, const int yPos, const bool makeWallsVisible, const bool isKnownOverride)
		: Coord(xPos, yPos), _display(as), _myColor(color), _isKnown(isKnownOverride), _canMove(true), _isTrap(false), _canSpawn(false)
	{
		initTraits(makeWallsVisible);
	}

	/** CONSTRUCTOR **
	 * Tile()
	 * Instantiate a blank tile with coordinate of (-1,-1) and no type.
	 */
	Tile() : Coord(-1, -1), _display(display::none), _isKnown(true), _canMove(false), _isTrap(false), _canSpawn(false) {}

	// Stream insertion operator
	friend std::ostream& operator<<(std::ostream& os, const Tile& t)
	{
		if ( t._myColor != WinAPI::color::white ) // if this tile has a color, set it
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<unsigned short>(t._myColor));
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
					for ( auto it : __VALID_TILES ) {
						if ( line.at(x) == static_cast<char>(it) )
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

class Cell final {
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
		if ( _max._y >= 10 && _max._x >= 10 ) {
			tRand rng;
			for ( auto y = 0; y < _max._y; y++ ) {
				std::vector<Tile> _row;
				for ( auto x = 0; x < _max._x; x++ ) {
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
	 * Generate a new cell with the given size parameters. Minimum size is 10x10
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
		_max(static_cast<long>(_matrix.at(0).size() - 1), static_cast<long>(_matrix.size() - 1)),
		isValidPos(_max)
	{
	}

	  /**
	   * getDisplayChar(Coord)
	   * Returns the display character of a given tile
	   *
	   * @param pos	 - Target position
	   * @returns char - ( ' ' for invalid position )
	   */
	char getDisplayChar(const Coord& pos)
	{
		if ( isValidPos(pos) )
			return static_cast<char>(get(pos)._display);
		return ' ';
	}

	/**
	 * modVis(bool)
	 * Modifies the visibility of all tiles in the cell.
	 *
	 * @param to		- ( true = visible ) ( false = invisible )
	 */
	void modVis(const bool to)
	{
		for ( auto& y : _matrix )
			for ( auto& x : y ) x._isKnown = to;
	}

	/**
	 * modVis(bool, Coord, const int)
	 * @brief Modifies the visibility of a square area around a given center-point in the cell.
	 *
	 * @param to		- ( true = visible ) ( false = invisible )
	 * @param pos		- The center-point
	 * @param radius	- The distance away from the center-point that will also be discovered.
	 */
	void modVis(const bool to, const Coord& pos, const int radius)
	{
		for ( int y = pos._y - radius; y <= pos._y + radius; y++ )
			for ( int x = pos._x - radius; x <= pos._x + radius; x++ )
				if ( isValidPos(x, y) )
					_matrix.at(y).at(x)._isKnown = to;
	}

	/**
	 * modVis(bool, Coord, const int)
	 * @brief Modifies the visibility of a circular area around a given center-point in the cell.
	 *
	 * @param to		- ( true = visible ) ( false = invisible )
	 * @param pos		- The center-point
	 * @param radius	- The distance away from the center-point that will also be discovered.
	 */
	void modVisCircle(const bool to, const Coord& pos, const int radius)
	{
		for ( int y = pos._y - radius; y <= pos._y + radius; y++ )
			for ( int x = pos._x - radius; x <= pos._x + radius; x++ )
				if ( isValidPos(x, y) && checkDistance::get_circle(x, y, pos, radius) )
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
	void modVis(const bool to, const Coord& minPos, const Coord& maxPos)
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
		for ( auto& y : _matrix ) {
			for ( auto& x : y ) buf << x;
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
	Tile &get(const int x, const int y)
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
	bool exportToFile(const std::string& filename, file::save_type saveAs = file::save_type::overwrite)
	{
		std::stringstream buf;
		for ( auto& y : _matrix ) {
			for ( auto& x : y ) buf << x;
			buf << std::endl;
		}
		return file::write(filename, buf, saveAs);
	}
};