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
	position& x{ first };
	position& y{ second };

	/**
	 * @brief		Constructor that accepts two positions.
	 * @param x		X-Axis (Horizontal/Column Index) Position.
	 * @param y		Y-Axis (Vertical/Row Index) Position.
	 */
	constexpr point(position&& x, position&& y) : std::pair<position, position>(std::forward<position>(x), std::forward<position>(y)) {}
	/**
	 * @brief		Constructor that accepts two positions.
	 * @param x		X-Axis (Horizontal/Column Index) Position.
	 * @param y		Y-Axis (Vertical/Row Index) Position.
	 */
	constexpr point(const position& x, const position& y) : std::pair<position, position>(x, y) {}

	/**
	 * @brief		Constructor that accepts two positions.
	 * @param p		Another point object.
	 */
	constexpr point(point&& p) noexcept : std::pair<position, position>(std::move(p)) {}
	/**
	 * @brief		Constructor that accepts two positions.
	 * @param p		Another point object.
	 */
	constexpr point(const point& p) : std::pair<position, position>(p) {}

	/**
	 * @brief	Default Constructor.
	 */
	constexpr point() : point(0ll, 0ll) {}

	/**
	 * @brief		Constructor that supports
	 * @param Tx	X-Axis (Horizontal/Column Index) Type.
	 * @param Ty	Y-Axis (Vertical/Row Index) Type.
	 * @param p		A std::pair rvalue.
	 */
	template<std::integral Tx, std::integral Ty> requires (!std::same_as<Tx, position> && !std::same_as<Ty, position>)
		constexpr point(std::pair<Tx, Ty>&& p) : std::pair<position, position>(static_cast<position>(p.first), static_cast<position>(p.second)) {}

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

	// comparison operators:

	bool operator<(const point& o) const
	{
		return ((x < o.x) && (y < o.y));
	}
	bool operator<=(const point& o) const
	{
		return ((x <= o.x) && (y <= o.y));
	}
	bool operator>(const point& o) const
	{
		return ((x > o.x) && (y > o.y));
	}
	bool operator>=(const point& o) const
	{
		return ((x >= o.x) && (y >= o.y));
	}
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
	point clamp() const
	{
		auto xP{ x }, yP{ y };
		if (bool xNegative{ x < 0 }; std::abs(x) > 1)
			xP = xNegative ? -1 : 1;
		if (bool yNegative{ y < 0 }; std::abs(y) > 1)
			yP = yNegative ? -1 : 1;
		return{ xP, yP };
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
	bool within(const point& min, const point& max) const
	{
		return x >= min.x && x < max.x&& y >= min.y && y < max.y;
	}
	/**
	 * @brief			Check if this point is within an arbitrary square region, from the top-left corner to the bottom-right corner.
	 * @param bounds	A pair of points where the first element is the minimum boundary and the second is the maximum boundary.
	 * @returns			bool
	 */
	bool within(const std::pair<point, point>& bounds) const
	{
		const auto& [min, max] { bounds };
		return within(min, max);
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

	/**
	 * @brief
	 * @param radius
	 * @param min
	 * @param max
	 * @returns			std::vector<point>
	 */
	std::vector<point> getAllPointsWithinCircle(const unsigned& radius, const point& min, const point& max, const bool& include_center = false) const
	{
		std::vector<point> vec;
		const auto& total_sz{ max - min };
		vec.reserve(total_sz.x & total_sz.y);

		const position r{ static_cast<position>(radius) }, r2{ r * r };

		for (position yP{ y - r }; yP <= y + r; ++yP) {
			if (yP < min.y)
				continue;
			else if (yP > max.y)
				break;
			for (position xP{ x - r }; xP <= x + r; ++xP) {
				if (xP < min.x)
					continue;
				else if (xP > max.x)
					break;
				const point& pos{ xP, yP };
				if ((include_center || !include_center && pos != *this) && withinCircle(radius, pos)) {
					vec.emplace_back(pos);
				}
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

	point north() const
	{
		return{ x, y - 1 };
	}
	point east() const
	{
		return{ x + 1, y };
	}
	point south() const
	{
		return{ x, y + 1 };
	}
	point west() const
	{
		return{ x - 1, y };
	}
	point northeast() const
	{
		return{ x + 1, y - 1 };
	}
	point northwest() const
	{
		return{ x - 1, y - 1 };
	}
	point southeast() const
	{
		return{ x + 1, y + 1 };
	}
	point southwest() const
	{
		return{ x - 1, y + 1 };
	}
	std::array<point, 4> cardinal() const
	{
		return{ north(), east(), south(), west() };
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

using size = point;
