#pragma once
#include <utility>

using position = long long;

struct point : std::pair<position, position> {
	position& x{ first };
	position& y{ second };
	constexpr point(position&& x, position&& y) : std::pair<position, position>(std::forward<position>(x), std::forward<position>(y)) {}
	constexpr point(const position& x, const position& y) : std::pair<position, position>(x, y) {}
	constexpr point(point&& p) noexcept : std::pair<position, position>(std::forward<point>(p)) {}
	constexpr point(const point& p) : std::pair<position, position>(p) {}
	constexpr point() : point(0ll, 0ll) {}
};
