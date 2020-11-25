/**
 * Coord.h
 * Contains the Coord struct, a wrapper for a single location in a matrix, or the size of a matrix.
 * by radj307
 */
#pragma once
#include <cmath>

/**
 * struct Coord
 * Wrapper for (x/y) positioning.
 * 
 * @param x	- Horizontal index
 * @param y	- Vertical index
 */
struct Coord {
	long _y;	// VERTICAL
	long _x;	// HORIZONTAL
	Coord(long x, long y) : _y(y), _x(x) {}
	Coord(const Coord &pos) : _y(pos._y), _x(pos._x) {}

	bool operator==(const Coord &o)
	{
		if ( _y == o._y && _x == o._x )
			return true;
		return false;
	}
	bool operator!=(const Coord &o)
	{
		if ( _y != o._y && _x != o._x )
			return true;
		return false;
	}
};

/** FUNCTOR **
 * struct checkDistance
 * Returns the distance between 2 given points. This functor does not have a constructor.
 */
struct checkDistance {
	// Returns the distance in tiles from pos1 to pos2
	long operator()(Coord pos1, Coord pos2)
	{
		return{ abs((pos1._x - pos2._x) + (pos1._y - pos2._y)) };
	}
	// Returns the distance in tiles from pos1 to pos2
	long operator()(long pos1X, long pos1Y, long pos2X, long pos2Y)
	{
		return{ abs((pos1X - pos2X) + (pos1Y - pos2Y)) };
	}
};

/** FUNCTOR **
 * struct checkDistanceFrom
 * Returns the distance between a given point and a member Coord pointer given in constructor
 */
struct checkDistanceFrom {
	// member coord pointer to follow, such as the player
	Coord* _follow{ nullptr };

	/** CONSTRUCTOR **
	 * checkDistanceFrom(Coord*)
	 * Instantiate a checkDistanceFrom functor with a pointer to a Coord instance
	 *
	 * @param followThis	- A pointer to a Coord instance to use when checking distance.
	 */
	checkDistanceFrom(Coord* followThis) : _follow(followThis) {}

	// Returns the distance in tiles from the given pos
	long operator()(Coord pos)
	{
		return{ abs((_follow->_x - pos._x) + (_follow->_y - pos._y)) };
	}
	// Returns the distance in tiles from the given pos
	long operator()(long posX, long posY)
	{
		return{ abs((_follow->_x - posX) + (_follow->_y - posY)) };
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
	checkBounds(const long maxPosX, const long maxPosY, const long minPosX = 0, const long minPosY = 0) : _maxPos(maxPosX, maxPosY), _minPos(minPosX, minPosY) {}

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
	bool operator()(long x, long y) const
	{
		if ( (y >= 0 && y < _maxPos._y) && (x >= 0 && x < _maxPos._x) )
			return true;
		return false;
	}
};