/**
 * world.h
 * Represents the game world.
 * Contains the Cell class, and the Tile struct.
 * by radj307
 */
#pragma once
#include <map>
#include <sstream>

#include "Coord.h"
#include "file.h"
#include "xRand.h"

struct TileAttrib {
	/**
	 * enum class display
	 * Defines valid tile types/display characters.
	 * Remember to add new entries to vector __VALID_TILE_TYPES below
	 */
	enum class display {
		empty = '_',
		wall = '#',
		hole = 'O',
		none = '?',
	};

	display _display;	// the display character
	bool _isKnown;		// true if this tile is known to the player
	bool _canMove;		// true if this tile allows actors to move to it
	bool _isTrap;		// true if this tile is a trap, and will damage actors when stepped on.
	bool _canSpawn;		// true if this tile allows actors to spawn on it

	explicit TileAttrib(const display displayChar = display::empty) :
		_display(displayChar),
		_isKnown(false),
		_canMove(_display == display::wall ? false : true),
		_isTrap(_display == display::hole ? true : false),
		_canSpawn(!_canMove || _isTrap ? false : true)
	{}
	explicit TileAttrib(const display displayChar, const bool isKnown) :
		_display(displayChar),
		_isKnown(isKnown),
		_canMove(_display == display::wall ? false : true),
		_isTrap(_display == display::hole ? true : false),
		_canSpawn(!_canMove || _isTrap ? false : true)
	{}
};

/**
 * struct Tile : public Coord
 * Represents a single position in the matrix of a cell
 */
struct Tile final : TileAttrib {
	
	Tile() = default;
	/** CONSTRUCTOR **
	 * Tile(TileAttrib::display)
	 * @brief Construct a tile with the default color.
	 * @param as				- This tile's type (display character)
	 * @param isKnownOverride	- Forces this tile to be visible/invisible from the start
	 */
	explicit Tile(const display as, const bool isKnownOverride = false) : TileAttrib(as, isKnownOverride) {}

	static Tile rng_build(const bool isKnownOverride, const bool makeWallsVisible, const float chance_wall = 9.0f, const float chance_hole = 5.0f)
	{
		tRand rng;
		auto rand{ rng.get(100.0f, 0.0f) };
		if ( rand <= chance_wall )
			return Tile{ display::wall, isKnownOverride };
		rand = rng.get(100.0f, 0.0f);
		if ( rand <= chance_hole )
			return Tile{ display::hole, isKnownOverride };
		return Tile{};
	}
	
	// Stream insertion operator
	friend std::ostream& operator<<(std::ostream& os, const Tile& t)
	{
		if ( t._isKnown ) // if this tile is known, insert its char
			os << static_cast<char>(t._display);
		else // if this tile is not known, insert a blank
			os << ' ';
		return os;
	}
};

class Cell final {
	using row = std::map<unsigned int, Tile>;
	using cell = std::map<unsigned int, row>;
	//using tileDefaults = std::map<TileAttrib::display, TileAttrib>;

	//tileDefaults _default_tiles;
	cell _cell;	// Tile matrix
	bool
		_vis_all,	// All tiles are always visible when true
		_vis_wall;	// Walls are always visible when true

	/**
	 * generate()
	 * @brief Generates the tile matrix using RNG
	 */
	void generate()
	{
		for ( auto y{ 0 }; y < _max._y; ++y ) {
			row r;
			for ( auto x{ 0 }; x < _max._x; ++x ) {
				if ( y == 0 || y == _max._y - 1 || x == 0 || x == _max._x - 1 )
					r.insert(std::make_pair(x, Tile{ Tile::display::wall, (_vis_wall ? true : false) }));
				else
					r.insert(std::make_pair(x, Tile::rng_build(_vis_all, _vis_wall)));
			}
			_cell.insert(std::make_pair(y, r));
		}
	}

public:
	// The Cell's Vertical & Horizontal size as a Coord
	const Coord _max;
	// Functor that checks if a given tile is within the cell boundaries
	const checkBounds isValidPos; // function syntax is used to emulate a member function

	/** CONSTRUCTOR **
	 * Cell(Coord, bool)
	 * @brief Generate a new cell with the given size parameters. Minimum size is 10x10
	 * @param cellSize				- The size of the cell
	 * @param makeWallsVisible		- walls are always visible
	 * @param override_known_tiles	- When true, all tiles will be visible to the player from the start.
	 */
	explicit Cell(const Coord& cellSize, const bool makeWallsVisible = true, const bool override_known_tiles = false) : _vis_all(override_known_tiles), _vis_wall(makeWallsVisible), _max(cellSize._x - 1, cellSize._y - 1), isValidPos(_max)
	{
		if ( _max._x < 10 || _max._y < 10 )
			throw std::exception("Cell size cannot be less than 10x10");
		generate();
	}

	/**
	 * getChar(Coord&)
	 * @brief Returns the display character of a given tile
	 * @param pos	 - Target position
	 * @returns char - ( ' ' for invalid position )
	 */
	char getChar(const Coord& pos)
	{
		if ( isValidPos(pos) )
			return static_cast<char>(_cell[pos._y][pos._x]._display);
		return ' ';
	}
	
	/**
	 * getChar(Coord&)
	 * @brief Returns the display character of a given tile
	 * 
	 * @returns char - ( ' ' for invalid position )
	 */
	char getChar(const unsigned int x, const unsigned int y)
	{
		if ( isValidPos(x, y) )
			return static_cast<char>(_cell[y][x]._display);
		return ' ';
	}

	/**
	 * modVis(bool)
	 * @brief Modifies the visibility of all tiles in the cell.
	 * @param to		- ( true = visible ) ( false = invisible )
	 */
	void modVis(const bool to)
	{
		if ( !_vis_all || to )
			for ( auto& [yIndex, row] : _cell )
				for ( auto& [xIndex, tile] : row ) {
					if ( tile._display == Tile::display::wall )
						tile._isKnown = to || _vis_wall;
					else
						tile._isKnown = to;
				}
	}

	/**
	 * modVis(bool, long, long)
	 * @brief Modifies the visibility of a given tile
	 * @param to		- ( true = visible ) ( false = invisible )
	 * @param x			- x-axis (horizontal) index.
	 * @param y			- y-axis (vertical) index.
	 */
	void modVis(const bool to, const long x, const long y)
	{
		if ( isValidPos(x, y) ) {
			if ( _cell[y][x]._display != Tile::display::wall )
				_cell[y][x]._isKnown = to || _vis_all;
			else
				_cell[y][x]._isKnown = to || _vis_wall;
		}
	}

	/**
	 * modVis(bool, Coord, int)
	 * @brief Modifies the visibility of a square area around a given center-point in the cell.
	 * @param to		- ( true = visible ) ( false = invisible )
	 * @param pos		- The center-point
	 * @param radius	- The distance away from the center-point that will also be discovered.
	 */
	void modVis(const bool to, const Coord& pos, const int radius)
	{
		if ( !_vis_all )
			for ( int y = pos._y - radius; y <= pos._y + radius; y++ )
				for ( int x = pos._x - radius; x <= pos._x + radius; x++ )
					modVis(to, x, y);
	}

	/**
	 * modVis(bool, Coord, const int)
	 * @brief Modifies the visibility of a circular area around a given center-point in the cell.
	 * @param to		- ( true = visible ) ( false = invisible )
	 * @param pos		- The center-point
	 * @param radius	- The distance away from the center-point that will also be discovered.
	 */
	void modVisCircle(const bool to, const Coord& pos, const int radius)
	{
		if ( !_vis_all || to )
			for ( int y = pos._y - radius; y <= pos._y + radius; y++ )
				for ( int x = pos._x - radius; x <= pos._x + radius; x++ )
					if ( checkDistance::get(x, y, pos, radius) )
						modVis(to, x, y);
	}

	/**
	 * modVis(bool, Coord, Coord)
	 * @brief Modifies the visibility of a specified area in the cell.
	 * @param to		- ( true = visible ) ( false = invisible )
	 * @param minPos	- The top-left corner of the target area
	 * @param maxPos	- The bottom-right corner of the target area
	 */
	void modVis(const bool to, const Coord& minPos, const Coord& maxPos)
	{
		if ( !_vis_all || to )
			for ( int y = minPos._y; y <= maxPos._y; y++ )
				for ( int x = minPos._x; x <= maxPos._x; x++ )
					modVis(to, x, y);
	}

	/**
	 * isTrap(Coord&)
	 * @brief Check if a tile is a trap
	 * @param pos	- Target position
	 * @returns bool
	 */
	bool isTrap(const Coord& pos)
	{
		return _cell[pos._y][pos._x]._isTrap;
	}
	
	/**
	 * isTrap(unsigned int, unsigned int)
	 * @brief Check if a tile is a trap
	 * @param x	- Target X position
	 * @param y	- Target Y position
	 * @returns bool
	 */
	bool isTrap(const unsigned int x, const unsigned int y)
	{
		return _cell[y][x]._isTrap;
	}

	/**
	 * canSpawn(Coord&)
	 * @brief Check if a tile allows actors to spawn
	 * @param pos	- Target position
	 * @returns bool
	 */
	bool canSpawn(const Coord& pos)
	{
		return _cell[pos._y][pos._x]._canSpawn;
	}
	
	/**
	 * canSpawn(unsigned int, unsigned int)
	 * @brief Check if a tile allows actors to spawn
	 * @param x	- Target X position
	 * @param y	- Target Y position
	 * @returns bool
	 */
	bool canSpawn(const unsigned int x, const unsigned int y)
	{
		return _cell[y][x]._canSpawn;
	}

	/**
	 * canMove(Coord&)
	 * @brief Check if a tile allows actors to move to it
	 * @param pos	- Target position
	 * @returns bool
	 */
	bool canMove(const Coord& pos)
	{
		return _cell[pos._y][pos._x]._canMove;
	}
	
	/**
	 * canMove(unsigned int, unsigned int)
	 * @brief Check if a tile allows actors to move to it
	 * @param x	- Target X position
	 * @param y	- Target Y position
	 * @returns bool
	 */
	bool canMove(const unsigned int x, const unsigned int y)
	{
		return _cell[y][x]._canMove;
	}

	/**
	 * isKnown(Coord&)
	 * @brief Check if a tile is visible to the player
	 * @param pos	- Target position
	 * @returns bool
	 */
	bool isKnown(const Coord& pos)
	{
		return _cell[pos._y][pos._x]._isKnown;
	}
	
	/**
	 * isKnown(unsigned int, unsigned int)
	 * @brief Check if a tile is visible to the player
	 * @param x	- Target X position
	 * @param y	- Target Y position
	 * @returns bool
	 */
	bool isKnown(const unsigned int x, const unsigned int y)
	{
		return _cell[y][x]._isKnown;
	}

	/**
	 * operator<<(ostream&, Cell&)
	 * @brief Stream insertion operator
	 * @param os	- Output stream ref
	 * @param c		- Cell instance ref
	 * @returns ostream&
	 */
	friend std::ostream& operator<<(std::ostream& os, Cell& c)
	{
		for ( auto& [yIndex, row] : c._cell ) {
			for ( auto& [xIndex, tile] : row )
				os << tile << ' ';
			os << std::endl;
		}
		return os;
	}

	/**
	 * exportToFile(string)
	 * @brief Exports this cell to a file.
	 * @param filename	- The name of the output file
	 * @returns bool	- ( true = success ) ( false = failed )
	 */
	bool exportToFile(const std::string& filename)
	{
		std::stringstream ss;
		ss << *this;
		return file::write(filename, ss, false);
	}
};