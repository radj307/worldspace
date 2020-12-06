/**
 * Coord.h
 * Contains the Coord struct, a wrapper for a single location in a matrix, or the size of a matrix.
 * by radj307
 */
#pragma once
#include <cmath>
#include <exception>

struct Coord {
	long _y{};	// VERTICAL
	long _x{};	// HORIZONTAL
	/** CONSTRUCTOR **
	 * Coord(long, long)
	 *
	 * @param x	- X-axis index.
	 * @param y	- Y-axis index.
	 */
	Coord(const long x, const long y) : _y(y), _x(x) {}
	/**
	 * Coord()
	 * @brief Default constructor with uninitialized values.
	 */
	Coord() = default;

	// Comparison operator
	bool operator==(const Coord &o) const
	{
		if ( _y == o._y && _x == o._x )
			return true;
		return false;
	}
	// Inverse comparison operator
	bool operator!=(const Coord &o) const
	{
		if ( _y != o._y && _x != o._x )
			return true;
		return false;
	}
};

/** FUNCTOR **
 * struct checkDistance
 * @brief Returns the distance between 2 given points. This functor does not have a constructor.
 */
struct checkDistance {
	/**
	 * get_circle(Coord&, Coord&, int)
	 * @brief Returns true if a given coordinate is within a circular radius of a given point
	 *
	 * @param pos		- The point to check
	 * @param center	- The centerpoint of the circle
	 * @param radius	- The radius of the circle
	 * @returns bool	- ( true = point is within circle ) ( false = point is not within circle )
	 */
	static bool get_circle(const Coord& pos, const Coord& center, const int radius)
	{
		return (pos._x - center._x) * (pos._x - center._x) + (pos._y - center._y) * (pos._y - center._y) <= radius * radius;
	}
	/**
	 * get_circle(Coord&, Coord&, int)
	 * @brief Returns true if a given coordinate is within a circular radius of a given point
	 *
	 * @param posX		- The X-axis index of the point to check
	 * @param posY		- The Y-axis index of the point to check
	 * @param center	- The centerpoint of the circle
	 * @param radius	- The radius of the circle
	 * @returns bool	- ( true = point is within circle ) ( false = point is not within circle )
	 */
	static bool get_circle(const int posX, const int posY, const Coord& center, const int radius)
	{
		return (posX - center._x) * (posX - center._x) + (posY - center._y) * (posY - center._y) <= radius * radius;
	}
	/**
	 * get(Coord&, Coord&)
	 * @brief Get the distance between 2 points. Always returns a positive value.
	 *
	 * @param pos1	- First position
	 * @param pos2	- Second position
	 * @returns long
	 */
	static long get(const Coord& pos1, const Coord& pos2)
	{
		return abs(pos1._x - pos2._x) + abs(pos1._y - pos2._y);
	}
	/**
	 * get(long, long, long, long)
	 * @brief Get the distance between 2 points. Always returns a positive value.
	 *
	 * @param pos1X - First position's X-axis
	 * @param pos1Y	- First position's Y-axis
	 * @param pos2X	- Second position's X-axis
	 * @param pos2Y	- Second position's Y-axis
	 * @returns long
	 */
	static long get(const long pos1X, const long pos1Y, const long pos2X, const long pos2Y)
	{
		return abs(pos1X - pos2X) + abs(pos1Y - pos2Y);
	}
	/**
	 * operator()
	 * @brief Returns the distance between 2 given points.
	 *
	 * @param pos1	- First point
	 * @param pos2	- Second point
	 * @returns long
	 */
	long operator()(const Coord& pos1, const Coord& pos2) const
	{
		return abs(pos1._x - pos2._x) + abs(pos1._y - pos2._y);
	}
	/**
	 * operator()
	 * @brief Returns the distance between 2 given points.
	 *
	 * @param pos1X	- First point's X (horizontal) index.
	 * @param pos1Y	- First point's Y (vertical) index.
	 * @param pos2X	- Second point's X (horizontal) index.
	 * @param pos2Y	- Second point's Y (vertical) index.
	 * @returns long
	 */
	long operator()(const long pos1X, const long pos1Y, const long pos2X, const long pos2Y) const
	{
		return abs(pos1X - pos2X) + abs(pos1Y - pos2Y);
	}
};

/** FUNCTOR **
 * struct checkDistanceFrom
 * @brief Returns the distance between a given point and a member Coord pointer given in constructor
 */
struct checkDistanceFrom {
	// member coord pointer to follow, such as the player
	Coord* _follow{ nullptr };

	/** CONSTRUCTOR **
	 * checkDistanceFrom(Coord*)
	 * @brief Instantiate a checkDistanceFrom functor with a pointer to a Coord instance
	 *
	 * @param followThis	- A pointer to a Coord instance to use when checking distance.
	 */
	explicit checkDistanceFrom(Coord& followThis) : _follow(&followThis)
	{
		if ( _follow == nullptr ) throw std::exception("checkDistanceFrom() exception: Given coord to follow was nullptr");
	}

	/**
	 * operator()
	 * @brief Returns the distance between this instance's coord pointer and a given point.
	 *
	 * @param pos	- Target position to check
	 * @returns long
	 */
	long operator()(const Coord& pos) const noexcept
	{
		return abs(_follow->_x - pos._x) + abs(_follow->_y - pos._y);
	}
	/**
	 * operator()
	 * @brief Returns the distance between this instance's coord pointer and a given point.
	 *
	 * @param posX	- Target X (horizontal) position to check
	 * @param posY	- Target Y (vertical) position to check
	 * @returns long
	 */
	long operator()(const long posX, const long posY) const noexcept
	{
		return abs(_follow->_x - posX) + abs(_follow->_y - posY);
	}
};

/** FUNCTOR **
 * struct checkBounds
 * @brief Used to check if a given Coord is within a boundary.
 */
struct checkBounds {
	// The max & min allowable positions
	Coord _maxPos, _minPos;

	/** CONSTRUCTOR **
	 * checkBounds(const Coord, const Coord)
	 *
	 * @param maxPos	- The maximum allowed x & y indexes
	 * @param minPos	- (Default: [0,0]) The minimum allowed x & y indexes
	 */
	explicit checkBounds(const Coord& maxPos, const Coord& minPos = Coord(0, 0)) : _maxPos(maxPos), _minPos(minPos) {}

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
	 * @brief Returns true if the given position is within boundaries
	 *
	 * @param pos		- Target position to check
	 * @returns bool
	 */
	bool operator()(const Coord& pos) const
	{
		if ( pos._y >= 0 && pos._y < _maxPos._y && (pos._x >= 0 && pos._x < _maxPos._x) )
			return true;
		return false;
	}

	/**
	 * operator()
	 * @brief Returns true if the given position is within boundaries
	 *
	 * @param x			- Target X (horizontal) position to check
	 * @param y			- Target Y (vertical) position to check
	 * @returns bool
	 */
	bool operator()(const long x, const long y) const
	{
		if ( y >= 0 && y < _maxPos._y && (x >= 0 && x < _maxPos._x) )
			return true;
		return false;
	}
};

/**
 * intToDir(int)
 * @brief Converts an integer to a direction char
 *
 * @param i		 - Input integer between 0 and 3
 * @returns char - ( 'w' == 0 ) ( 'd' == 1 ) ( 's' == 2 ) ( 'a' == 3 ) ( ' ' == invalid parameter )
 */
inline char intToDir(const int i)
{
	switch ( i ) {
	case 0: return 'w';
	case 1: return 'd';
	case 2: return 's';
	case 3: return 'a';
	default:return ' ';
	}
}

/**
 * intToDir(int)
 * @brief Converts an integer to a direction char
 *
 * @param c		 - Input integer between 0 and 3
 * @returns int - ( 0 == 'w' ) ( 1 == 'd' ) ( 2 == 's' ) ( 3 == 'a' ) ( -1 == invalid parameter )
 */
inline int dirToInt(const char c)
{
	switch ( c ) {
	case 'w': return 0;
	case 'd': return 1;
	case 's': return 2;
	case 'a': return 3;
	default:return -1;
	}
}
