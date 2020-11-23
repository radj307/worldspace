/**
 * Coord.h
 * Contains the Coord struct, a wrapper for a single location in a matrix, or the size of a matrix.
 * by radj307
 */
#pragma once

/**
 * struct Coord
 * Wrapper for (x/y) positioning.
 * 
 * @param x	- Horizontal index
 * @param y	- Vertical index
 */
struct Coord {
	size_t _y;	// VERTICAL
	size_t _x;	// HORIZONTAL
	Coord(size_t x, size_t y) : _y(y), _x(x) {}
	Coord(const Coord &pos) : _y(pos._y), _x(pos._x) {}
};