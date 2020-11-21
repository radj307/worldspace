#pragma once
#include <iostream>
#include <vector>
#include <sstream>
#include "xRand.h"

struct Coord {
	size_t _y;	// VERTICAL
	size_t _x;	// HORIZONTAL
	Coord(size_t x, size_t y) : _y(y), _x(x) {}
	Coord(const Coord& pos) : _y(pos._y), _x(pos._x) {}
	Coord(Coord&& pos) noexcept : _y(std::move(pos._y)), _x(std::move(pos._x)) {}
};

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
	// a vector of 
	std::vector<std::vector<Tile>> _matrix;

protected:

	// build horizontal, then vertical
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
	const unsigned int _sizeV, _sizeH;

	Cell(Coord cellSize, bool override_known_tiles = false) : _sizeH(cellSize._x), _sizeV(cellSize._y)
	{
		generate(override_known_tiles);
	}

	// display horizontal for each vertical
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

	// discovers a square of radius size around the given pos
	inline void discover(Coord pos, const int radius = 1)
	{
		for ( unsigned int y = (pos._y - radius); y <= (pos._y + radius); y++ ) {
			for ( unsigned int x = (pos._x - radius); x <= (pos._x + radius); x++ ) {
				if ( (y >= 0 && y < _matrix.size()) && (x >= 0 && x < _matrix.at(y).size()) )
					_matrix.at(y).at(x)._isKnown = true;
			}
		}
	}

	// discovers the whole map
	inline void discover()
	{
		for ( auto y = _matrix.begin(); y != _matrix.end(); y++ ) {
			for ( auto x = y->begin(); x != y->end(); x++ ) {
				x->_isKnown = true;
			}
		}
	}

	inline Tile& get(Coord pos, const bool findByIndex = false)
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