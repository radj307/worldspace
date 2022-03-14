#pragma once
#include <utility>

using position = long long;

struct point : std::pair<position, position> {
	position& x{ first };
	position& y{ second };
	constexpr point(position&& x, position&& y) : std::pair<position, position>(std::forward<position>(x), std::forward<position>(y)) {}
	constexpr point(const position& x, const position& y) : std::pair<position, position>(x, y) {}
	constexpr point(point&& p) noexcept : std::pair<position, position>(std::move(p)) {}
	constexpr point(const point& p) : std::pair<position, position>(p) {}
	constexpr point() : point(0ll, 0ll) {}

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

	template<std::integral T>
	point operator-(const T& n) const
	{
		return{ x - n, y - n };
	}
	template<std::integral T>
	point& operator-=(const T& n)
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
	template<std::integral T>
	point operator+(const T& n) const
	{
		return{ x + n, y + n };
	}
	template<std::integral T>
	point& operator+=(const T& n)
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
	template<std::integral T>
	point operator*(const T& n) const
	{
		return{ x * n, y * n };
	}
	template<std::integral T>
	point& operator*(const T& n)
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
	template<std::integral T>
	point operator/(const T& n) const
	{
		return{ x / n, y / n };
	}
	template<std::integral T>
	point& operator/(const T& n)
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

	point distanceTo(const point& o) const
	{
		return *this - o;
	}

	bool within(const point& min, const point& max) const
	{
		return x >= min.x && x < max.x&& y >= min.y && y < max.y;
	}
};
