#pragma once
#include <concepts>
#include <utility>

template<std::integral T>
struct custom_point : std::pair<T, T> {
	T& x{ first };
	T& y{ second };
	point(T&& x, T&& y) : std::pair<T, T>(std::forward<T>(x), std::forward<T>(y)) {}
	point() : point(static_cast<T>(0), static_cast<T>(0)) {}
};

// @brief	Standardized position type that applies to a single axis.
using pos = long long;
// @brief	Standardized point type that applies to a point in a matrix.
using point = custom_point<pos>;
