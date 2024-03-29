/**
 * Coord.h
 * Contains the Coord struct, a wrapper for a single location in a matrix, or the size of a matrix.
 * @author radj307
 */
#pragma once
#include <cmath>
#include <exception>

#include "sysapi.h"

// This must be set to an unused value for internal error checking to work correctly
constexpr auto __NULL_COORD_VAL = -1L;

/**
 * struct Coord
 * @brief Wrapper for a location in a matrix, with an X & Y index.
 */
struct Coord {
	long
		_y{},	// VERTICAL
		_x{};	// HORIZONTAL
	
	/**
	 * Coord(long, long)
	 * @brief Constructor
	 * @param x	- X-axis (horizontal) index.
	 * @param y	- Y-axis (vertical) index.
	 */
	Coord(const long x, const long y) : _y(y), _x(x) {}
	
	/**
	 * Coord()
	 * @brief Default constructor with null values.
	 */
	Coord() : _y(__NULL_COORD_VAL), _x(__NULL_COORD_VAL) {}

	/**
	 * set(TyX, TyY)
	 * @brief Change the values of this coord
	 * @tparam TyX	- Type of param x
	 * @tparam TyY	- Type of param y
	 * @param x		- Horizontal X-axis value
	 * @param y		- Vertical Y-axis value
	 * @returns bool
	 */
	template<typename TyX = long, typename TyY = long>
	bool set(TyX x, TyY y)
	{
		try {
			_x = static_cast<long>(x);
			_y = static_cast<long>(y);
			return true;
		} catch ( ... ) { return false; }
	}

	// Comparison operator
	bool operator==(const Coord &o) const { return _y == o._y && _x == o._x; }
	// Inverse comparison operator
	bool operator!=(const Coord &o) const { return _y != o._y && _x != o._x; }
};

static const Coord _NULL_COORD;


namespace sys {
	/** // Add an overload to the cursorPos function so Coord can be passed as parameter:
	 * cursorPos(Coord)
	 * @brief Sets the cursor's position to a given x/y coordinate, in relation to the origin point top left corner (0,0)
	 * @param pos		- Target position, measured in characters of the screen buffer
	 * @returns bool	- ( true = success ) ( false = failed )
	 */
	static bool cursorPos(const Coord& pos) { return sys::cursorPos(pos._x, pos._y); }
}

/** FUNCTOR **
 * struct checkDistance
 * @brief Returns the distance between 2 given points. This functor does not have a constructor.
 */
struct checkDistance final {
	/**
	 * get(Coord&, Coord&, int)
	 * @brief Returns true if a given coordinate is within a circular radius of a given point
	 * @param pos		- The point to check
	 * @param center	- The centerpoint of the circle
	 * @param radius	- The radius of the circle
	 * @returns bool	- ( true = point is within circle ) ( false = point is not within circle )
	 */
	template < typename Int > static bool get( const Coord& pos, const Coord& center, const Int& radius ) { return ( pos._x - center._x ) * ( pos._x - center._x ) + ( pos._y - center._y ) * ( pos._y - center._y ) <= radius * radius; }
	/**
	 * get(Coord&, Coord&, int)
	 * @brief Returns true if a given coordinate is within a circular radius of a given point
	 * @param posX		- The X-axis index of the point to check
	 * @param posY		- The Y-axis index of the point to check
	 * @param center	- The centerpoint of the circle
	 * @param radius	- The radius of the circle
	 * @returns bool	- ( true = point is within circle ) ( false = point is not within circle )
	 */
	template < typename Int > static bool get( const int& posX, const int& posY, const Coord& center, const Int& radius ) { return ( posX - center._x ) * ( posX - center._x ) + ( posY - center._y ) * ( posY - center._y ) <= radius * radius; }
	/**
	 * get(Coord&, Coord&)
	 * @brief Get the distance between 2 points. Always returns a positive value.
	 * @param pos1	- First position
	 * @param pos2	- Second position
	 * @returns long
	 */
	static long get(const Coord& pos1, const Coord& pos2) { return abs(pos1._x - pos2._x) + abs(pos1._y - pos2._y); }
	/**
	 * get(long, long, long, long)
	 * @brief Get the distance between 2 points. Always returns a positive value.
	 * @param pos1X - First position's X-axis
	 * @param pos1Y	- First position's Y-axis
	 * @param pos2X	- Second position's X-axis
	 * @param pos2Y	- Second position's Y-axis
	 * @returns long
	 */
	static long get(const long pos1X, const long pos1Y, const long pos2X, const long pos2Y) { return abs(pos1X - pos2X) + abs(pos1Y - pos2Y); }
	/**
	 * operator()
	 * @brief Returns the distance between 2 given points.
	 * @param pos1	- First point
	 * @param pos2	- Second point
	 * @returns long
	 */
	long operator()(const Coord& pos1, const Coord& pos2) const { return abs(pos1._x - pos2._x) + abs(pos1._y - pos2._y); }
	/**
	 * operator()
	 * @brief Returns the distance between 2 given points.
	 * @param pos1X	- First point's X (horizontal) index.
	 * @param pos1Y	- First point's Y (vertical) index.
	 * @param pos2X	- Second point's X (horizontal) index.
	 * @param pos2Y	- Second point's Y (vertical) index.
	 * @returns long
	 */
	long operator()(const long pos1X, const long pos1Y, const long pos2X, const long pos2Y) const { return abs(pos1X - pos2X) + abs(pos1Y - pos2Y); }
};

/** FUNCTOR **
 * struct checkDistanceFrom
 * @brief Specialized variant of checkDistance that maintains a pointer to a single Coord instance, and checks the distance between it and a given point.
 */
struct checkDistanceFrom final {
	// member coord pointer to follow, such as the player
	Coord* _follow{ nullptr };

	/** CONSTRUCTOR **
	 * checkDistanceFrom(Coord*)
	 * @brief Instantiate a checkDistanceFrom functor with a pointer to a Coord instance
	 *
	 * @param followThis	- A pointer to a Coord instance to use when checking distance.
	 */
	explicit checkDistanceFrom(Coord& followThis) : _follow(&followThis) { if ( _follow == nullptr ) throw std::exception("checkDistanceFrom() exception: Given coord to follow was nullptr"); }

	/**
	 * operator()
	 * @brief Returns the distance between this instance's coord pointer and a given point.
	 *
	 * @param pos	- Target position to check
	 * @returns long
	 */
	long operator()(const Coord& pos) const noexcept { return abs(_follow->_x - pos._x) + abs(_follow->_y - pos._y); }
	/**
	 * operator()
	 * @brief Returns the distance between this instance's coord pointer and a given point.
	 *
	 * @param posX	- Target X (horizontal) position to check
	 * @param posY	- Target Y (vertical) position to check
	 * @returns long
	 */
	long operator()(const long posX, const long posY) const noexcept { return abs(_follow->_x - posX) + abs(_follow->_y - posY); }
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
		return pos._y >= 0 && pos._y < _maxPos._y && (pos._x >= 0 && pos._x < _maxPos._x);
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
		return y >= 0 && y < _maxPos._y && (x >= 0 && x < _maxPos._x);
	}
};