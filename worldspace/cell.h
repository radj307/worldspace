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
#include "xRand.h"
#include "Coord.h"

struct Tile : public Coord {
	enum class display {
		none = '?',
		empty = '_',
		wall = '#',
	};
	display _char;
	bool _isKnown;
	Tile(display as, int xPos, int yPos, bool isKnownOverride = false) : Coord(xPos, yPos), _char(as), _isKnown(isKnownOverride) {}
	Tile() : Coord(-1, -1), _char(display::none), _isKnown(true) {}

	friend inline std::ostream& operator<<(std::ostream& os, const Tile& t)
	{
		if ( t._isKnown )
			os << char(t._char) << ' ';
		else
			os << ' ' << ' ';
		return os;
	}
};

class Cell {
	Tile error;
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

			for ( unsigned int y = 0; y < _sizeV; y++ ) {
				std::vector<Tile> _row;
				for ( unsigned int x = 0; x < _sizeH; x++ ) {
					// make walls on all edges
					if ( (x == 0 || x == (_sizeH - 1)) || (y == 0 || y == (_sizeV - 1)) )
						_row.push_back(Tile(Tile::display::wall, x, y, override_known_tiles));
					else { // not an edge
						if ( rng.get(100u, 0u) < 10 ) {
							_row.push_back(Tile(Tile::display::wall, x, y, override_known_tiles));
						}
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
	const unsigned int _sizeV, _sizeH;

	/** CONSTRUCTOR **
	 * Cell(Coord, bool)
	 * 
	 * @param cellSize				- The size of the cell
	 * @param override_known_tiles	- When true, all tiles will be visible to the player from the start.
	 */
	Cell(Coord cellSize, bool override_known_tiles = false) : _sizeH(cellSize._x), _sizeV(cellSize._y)
	{
		generate(override_known_tiles);
	}

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
	 * discover(Coord, const int)
	 * Allows the player to see a square part of the map.
	 * 
	 * @param pos		- The center-point
	 * @param radius	- The distance away from the center-point that will also be discovered.
	 */
	inline void discover(Coord pos, const int radius = 1)
	{
		for ( unsigned int y = (pos._y - radius); y <= (pos._y + radius); y++ ) {
			for ( unsigned int x = (pos._x - radius); x <= (pos._x + radius); x++ ) {
				if ( (y >= 0 && y < _matrix.size()) && (x >= 0 && x < _matrix.at(y).size()) )
					_matrix.at(y).at(x)._isKnown = true;
			}
		}
	}

	/**
	 * discover()
	 * Allows the player to see the entire map.
	 */
	inline void discover()
	{
		for ( auto y = _matrix.begin(); y != _matrix.end(); y++ ) {
			for ( auto x = y->begin(); x != y->end(); x++ ) {
				x->_isKnown = true;
			}
		}
	}


	/**
	 * get(Coord, const bool)
	 * Returns a reference to the target tile.
	 * 
	 * @param pos			- The target tile
	 * @param findByIndex	- (Default: false) Whether to search the matrix from pos (0,0 - true) or (1,1 - false)
	 */
	inline Tile &get(Coord pos, const bool findByIndex = false)
	{
		switch ( findByIndex ) {
		case true:
			if ( (pos._x >= 0 && pos._x < _sizeH) && (pos._y >= 0 && pos._y < _sizeV) ) {
				return _matrix.at(pos._y).at(pos._x);
			}
			else break;
		default:
			if ( (pos._x >= 0 && pos._x <= _sizeH) && (pos._y >= 0 && pos._y <= _sizeV) ) {
				return _matrix.at(pos._y - 1).at(pos._x - 1);
			}
			else break;
		}
		return error;
	}
};