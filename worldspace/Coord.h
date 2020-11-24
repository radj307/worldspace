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

	bool operator==(Coord &o)
	{
		if ( _y == o._y && _x == o._x )
			return true;
		return false;
	}
};

/** FUNCTOR **
 * struct checkBounds
 * Used to check if a given Coord is within a boundary.
 */
struct checkBounds {
	// The max & min allowable positions 
	const Coord _maxPos, _minPos;

	/** CONSTRUCTOR **
	 * checkBounds(const Coord, const Coord)
	 * 
	 * @param maxPos	- The maximum allowed x & y indexes
	 * @param minPos	- (Default: [0,0]) The minimum allowed x & y indexes
	 */
	checkBounds(const Coord maxPos, const Coord minPos = Coord(0, 0)) : _maxPos(maxPos), _minPos(minPos) {}

	/** CONSTRUCTOR **
	 * checkBounds(const Coord, const Coord)
	 *
	 * @param maxPosX	- The maximum allowed X index.
	 * @param maxPosY	- The maximum allowed Y index.
	 * @param minPosX	- (Def: 0) The minimum allowed X index.
	 * @param minPosY	- (Def: 0) The minimum allowed Y index.
	 */
	checkBounds(const size_t maxPosX, const size_t maxPosY, const size_t minPosX = 0, const size_t minPosY = 0) : _maxPos(maxPosX, maxPosY), _minPos(minPosX, minPosY) {}

	/**
	 * operator()  
	 * Returns true if the given position is within boundaries
	 * 
	 * @param pos		- Target position to check
	 */
	bool operator()(Coord pos) const
	{
		if ( (pos._y >= 0 && pos._y < _maxPos._y) && (pos._x >= 0 && pos._x < _maxPos._x) )
			return true;
		return false;
	}

	/**
	 * operator()
	 * Returns true if the given position is within boundaries
	 *
	 * @param x			- Target X (horizontal) position to check
	 * @param y			- Target Y (vertical) position to check
	 */
	bool operator()(size_t x, size_t y) const
	{
		if ( (y >= 0 && y < _maxPos._y) && (x >= 0 && x < _maxPos._x) )
			return true;
		return false;
	}
};