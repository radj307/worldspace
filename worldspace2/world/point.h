#pragma once
#include <var.hpp>
#include <math.hpp>

#include <utility>
#include <array>

/// @brief	A 1-Dimensional position on a line.
using position = int;

/**
 * @struct	point
 * @brief	A 2-Dimensional point in a matrix.
 */
struct point : std::pair<position, position> {
	using base = std::pair<position, position>;

	position& x;
	position& y;

	/**
	 * @brief		Constructor that accepts two positions.
	 * @param x		X-Axis (Horizontal/Column Index) Position.
	 * @param y		Y-Axis (Vertical/Row Index) Position.
	 */
	constexpr point(position&& x, position&& y) : base(std::forward<position>(x), std::forward<position>(y)), x{ first }, y{ second } {}
	/**
	 * @brief		Constructor that accepts two positions.
	 * @param x		X-Axis (Horizontal/Column Index) Position.
	 * @param y		Y-Axis (Vertical/Row Index) Position.
	 */
	constexpr point(const position& x, const position& y) : base(x, y), x{ first }, y{ second } {}

	/**
	 * @brief		Constructor that accepts two positions.
	 * @param p		Another point object.
	 */
	constexpr point(point&& p) noexcept : base(std::move(p)), x{ first }, y{ second } {}
	/**
	 * @brief		Constructor that accepts two positions.
	 * @param p		Another point object.
	 */
	constexpr point(const point& p) : base(p), x{ first }, y{ second } {}

	/**
	 * @brief	Default Constructor.
	 */
	constexpr point() : point(static_cast<position>(0), static_cast<position>(0)) {}

	/**
	 * @brief		Constructor that supports
	 * @param Tx	X-Axis (Horizontal/Column Index) Type.
	 * @param Ty	Y-Axis (Vertical/Row Index) Type.
	 * @param p		A std::pair rvalue.
	 */
	template<std::integral Tx, std::integral Ty> requires (!std::same_as<Tx, position> && !std::same_as<Ty, position>)
		constexpr point(std::pair<Tx, Ty>&& p) : base(static_cast<position>(p.first), static_cast<position>(p.second)), x{ first }, y{ second } {}

	point& operator=(point&& o) noexcept
	{
		x = std::move(o.x);
		y = std::move(o.y);
		return *this;
	}
	point& operator=(const point& o)
	{
		x = o.x;
		y = o.y;
		return *this;
	}

	template<int I> auto const& get() const&
	{
		if constexpr (I == 0) return x;
		else if constexpr (I == 1) return y;
	}
	template<int I> auto& get()&
	{
		if constexpr (I == 0) return x;
		else if constexpr (I == 1) return y;
	}
	template<int I> auto&& get()&&
	{
		if constexpr (I == 0) return std::move(x);
		else if constexpr (I == 1) return std::move(y);
	}

	// negation operators:

	point operator-() const
	{
		return{ -x, -y };
	}
	point& operator-()
	{
		x = -x;
		y = -y;
		return *this;
	}

	// subtraction operators:

	template<std::integral T> point operator-(const T& n) const
	{
		return{ x - n, y - n };
	}
	template<std::integral T> point& operator-=(const T& n)
	{
		x -= n;
		y -= n;
		return *this;
	}
	point operator-(const point& o) const
	{
		return{ x - o.x, y - o.y };
	}
	point& operator-=(const point& o)
	{
		x -= o.x;
		y -= o.y;
		return *this;
	}

	// addition operators:

	template<std::integral T> point operator+(const T& n) const
	{
		return{ x + n, y + n };
	}
	template<std::integral T> point& operator+=(const T& n)
	{
		x += n;
		y += n;
		return *this;
	}
	point operator+(const point& o) const
	{
		return{ x + o.x, y + o.y };
	}
	point& operator+=(const point& o)
	{
		x += o.x;
		y += o.y;
		return *this;
	}

	// multiplication operators:

	template<std::integral T> point operator*(const T& n) const
	{
		return{ x * n, y * n };
	}
	template<std::integral T> point& operator*(const T& n)
	{
		x *= n;
		y *= n;
		return *this;
	}
	point operator*(const point& o) const
	{
		return{ x * o.x, y * o.y };
	}
	point& operator*=(const point& o)
	{
		x *= o.x;
		y *= o.y;
		return *this;
	}

	// division operators:

	template<std::integral T> point operator/(const T& n) const
	{
		return{ x / n, y / n };
	}
	template<std::integral T> point& operator/(const T& n)
	{
		x /= n;
		y /= n;
		return *this;
	}
	point operator/(const point& o) const
	{
		return{ x / o.x, y / o.y };
	}
	point& operator/(const point& o)
	{
		x /= o.x;
		y /= o.y;
		return *this;
	}

	// modulo operators:

	template<std::integral T> point operator%(const T& n) const
	{
		return{ x % n, y % n };
	}
	template<std::integral T> point& operator%=(const T& n)
	{
		x %= n;
		y %= n;
		return *this;
	}
	point operator%(const point& o) const
	{
		return{ x % o.x, y % o.y };
	}
	point& operator%=(const point& o)
	{
		x %= o.x;
		y %= o.y;
		return *this;
	}

	// comparison operators: (note that less/greater than operators are not defined on purpose - the compiler generates better ones using hashing)

	bool operator==(const point& o) const
	{
		return ((x == o.x) && (y == o.y));
	}
	bool operator!=(const point& o) const
	{
		return ((x != o.x) || (y != o.y));
	}

	/**
	 * @brief		Retrieve the distance between this point and another, by subtracting the given point from this point.
	 * @param o		Point to calculate the distance to.
	 * @returns		point
	 */
	point distanceTo(const point& o) const
	{
		return o - *this;
	}
	/**
	 * @brief		Retrieve the distance between this point an another, as an absolute 1-dimensional distance number.
	 * @param o		Point to calculate the distance to.
	 * @returns		position
	 */
	position directDistanceTo(const point& o) const
	{
		const auto& dist{ distanceTo(o) };
		return math::abs(dist.x + dist.y);
	}

	position getLargestAxis() const
	{
		return std::abs(x) > std::abs(y) ? x : y;
	}
	position getSmallestAxis() const
	{
		return std::abs(x) < std::abs(y) ? x : y;
	}
	bool equalAxis() const
	{
		return std::abs(x) == std::abs(y);
	}

	static position clamp(const position& v)
	{
		if (v == 0)
			return v;
		else if (v < 0)
			return -1;
		else return 1;
	}

	point clamp() const { return{ clamp(x), clamp(y) }; }

	/**
	 * @brief					This variant of the clamp() method will always return at least 1 zeroed axis.
	 * @param zeroLargerAxis	When the X & Y axis are NOT equal distances from 0, determines whether the larger or smaller axis is returned.
	 *\n						 true	- Selects the closer axis.
	 *\n						 false	- Selects the further axis.
	 * @param ifSameUseX		When true and if (x == y), the Y axis is zeroed and the X axis is returned.
	 * @returns	point			A point with 1 (or 2 if both are already zero) axis set to zero, and the other clamped between -1 and 1.
	 */
	point pathClamp(const bool& zeroLargerAxis = true, const bool& ifSameUseX = true) const
	{
		// one axis is already zeroed
		if (x == 0 || y == 0)
			return clamp();
		// x axis and y axis are equal distances from 0
		else if (const position absX{ std::abs(x) }, absY{ std::abs(y) }; absX == absY)
			return (ifSameUseX ? point{ clamp(x), 0 } : point{ 0, clamp(y) });
		// zero the axis that is further from 0
		else if (zeroLargerAxis)
			return (absX > absY ? point{ 0, clamp(y) } : point{ clamp(x), 0 });
		// zero the axis that is closer to 0
		else return (absX > absY ? point{ clamp(x), 0 } : point{ 0, clamp(y) });
	}

	point zeroedLargestAxis() const
	{
		return (x > y
			? point{ 0, y }
			: point{ x, 0 }
		);
	}
	point zeroedSmallestAxis() const
	{
		return (x > y
			? point{ 0, y }
			: point{ x, 0 }
		);
	}
	void zeroLargestAxis()
	{
		if (x > y)
			x = 0;
		else
			y = 0;
	}
	void zeroSmallestAxis()
	{
		if (x < y)
			x = 0;
		else
			y = 0;
	}
	point& swap()
	{
		const auto& copyY{ y };
		y = x;
		x = copyY;
		return *this;
	}

	/**
	 * @brief		Check if this point is within an arbitrary square region, from the top-left corner (min) to the bottom-right corner (max).
	 * @param min	Minimum point.
	 * @param max	Maximum point.
	 * @returns		bool
	 */
	bool withinSquare(const point& min, const point& max) const
	{
		return x >= min.x && x < max.x&& y >= min.y && y < max.y;
	}
	/**
	 * @brief			Check if this point is within an arbitrary square region, from the top-left corner to the bottom-right corner.
	 * @param bounds	A pair of points where the first element is the minimum boundary and the second is the maximum boundary.
	 * @returns			bool
	 */
	bool withinSquare(const std::pair<point, point>& bounds) const
	{
		return withinSquare(bounds.first, bounds.second);
	}

	bool withinCircle(const unsigned& radius, const point& pos) const
	{
		auto x{ pos.x - this->x }, y{ pos.y - this->y };
		x *= x;
		y *= y;
		return x + y <= (radius * radius);
	}

	bool withinCircle(const unsigned& radius, const position& xPos, const position& yPos) const
	{
		return withinCircle(radius, point{ xPos, yPos });
	}

	std::vector<point> getAllPointsWithinCircle(const unsigned& radius, const point& min, const point& max, const bool& include_center = false) const
	{
		std::vector<point> vec;
		const point& total_sz{ max - min };
		vec.reserve(static_cast<size_t>(total_sz.x * total_sz.y));

		const position r{ static_cast<position>(radius) }, r2{ r * r };

		// iterate on the vertical axis' tangential square (square drawn around the circle, with all of its sides touching the outer edges of the circle)
		for (position yP{ y - r }; yP <= y + r; ++yP) {
			if (yP < min.y)
				continue; // we're outside of the boundary, but may re-enter it in subsequent iterations.
			else if (yP > max.y)
				break; // we've exceeded the boundary, and won't be re-entering it.

			// iterate on the horizontal axis' tangential square
			for (position xP{ x - r }; xP <= x + r; ++xP) {
				if (xP < min.x)
					continue; // we're outside of the boundary, but may re-enter it in subsequent iterations.
				else if (xP > max.x)
					break; // we've exceeded the boundary, and won't be re-entering it.
				else if (!include_center && xP == x && yP == y)
					continue;

				if (withinCircle(radius, xP, yP))
					vec.emplace_back(std::move(point{ xP, yP }));
			}
		}

		vec.shrink_to_fit();
		return vec;
	}
	std::vector<point> getAllPointsWithinCircle(const unsigned& radius, const std::pair<point, point>& bounds, const bool& include_center = false) const
	{
		const auto& [min, max] { bounds };
		return getAllPointsWithinCircle(radius, min, max, include_center);
	}

	/// @brief	Retrieve the point that is 1 position north of this one. (top-left origin)
	point north() const
	{
		return{ x, y - 1 };
	}
	/// @brief	Retrieve the point that is 1 position east of this one. (top-left origin)
	point east() const
	{
		return{ x + 1, y };
	}
	/// @brief	Retrieve the point that is 1 position south of this one. (top-left origin)
	point south() const
	{
		return{ x, y + 1 };
	}
	/// @brief	Retrieve the point that is 1 position west of this one. (top-left origin)
	point west() const
	{
		return{ x - 1, y };
	}
	/// @brief	Retrieve the point that is 1 position north & 1 position east of this one. (top-left origin)
	point northeast() const
	{
		return{ x + 1, y - 1 };
	}
	/// @brief	Retrieve the point that is 1 position north & 1 position west of this one. (top-left origin)
	point northwest() const
	{
		return{ x - 1, y - 1 };
	}
	/// @brief	Retrieve the point that is 1 position south & 1 position east of this one. (top-left origin)
	point southeast() const
	{
		return{ x + 1, y + 1 };
	}
	/// @brief	Retrieve the point that is 1 position south & 1 position west of this one. (top-left origin)
	point southwest() const
	{
		return{ x - 1, y + 1 };
	}
	/// @brief	Retrieve an array of the points that are 1 position away in each of the 4 cardinal directions. Ordering is [NESW].
	std::array<point, 4> cardinal() const
	{
		return{ north(), east(), south(), west() };
	}

	point slideX(const int& count, const bool& zeroY = false)
	{
		return{ x + count, zeroY ? 0 : y };
	}
	point slideY(const int& count, const bool& zeroX = false) const
	{
		return{ zeroX ? 0 : x, y + count };
	}
};

namespace std {
	// Add tuple_size overload to std namespace to support structured binding decomposition
	template<> struct tuple_size<point> : std::integral_constant<position, 2> {};
	// Add tuple_element overload to std namespace to support structured binding decomposition
	template<> struct tuple_element<0, point> { using type = position; };
	template<> struct tuple_element<1, point> { using type = position; };
}

/**
 * @brief	Retrieve a point with the smaller X & Y values of each axis.
 * @param l 1st Point.
 * @param r	2nd Point.
 * @returns	point
 */
inline static point minimize(const point& l, const point& r)
{
	return{ l.x < r.x ? l.x : r.x, l.y < r.y ? l.y : r.y };
}

/**
 * @brief	Retrieve a point with the larger X & Y values of each axis.
 * @param l 1st Point.
 * @param r	2nd Point.
 * @returns	point
 */
inline static point maximize(const point& l, const point& r)
{
	return{ l.x > r.x ? l.x : r.x, l.y > r.y ? l.y : r.y };
}

inline static std::vector<point> getRelativeDiff(const point& direction) noexcept(false)
{
	if (direction.x != 0 && direction.y != 0)
		throw make_exception("Invalid cardinal direction point (", direction.x, ", ", direction.y, ") cannot have 2 non-zero axis!");
	std::vector<point> vec;
	vec.reserve(2);
	if (direction.x == 0) {
		vec.emplace_back(point{ -1, 0 });
		vec.emplace_back(point{ 1, 0 });
	}
	else if (direction.y == 0) {
		vec.emplace_back(point{ 0, -1 });
		vec.emplace_back(point{ 0, 1 });
	}
	vec.shrink_to_fit();
	return vec;
}

using size = point;
