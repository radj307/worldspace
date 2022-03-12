#pragma once
#include "../world/point.h"

#include <TermAPI.hpp>
#include <var.hpp>

struct DisplayableBase {
	char display;
	color::setcolor color;

	WINCONSTEXPR DisplayableBase(const char& display, const color::setcolor& color) : display{ display }, color{ color } {}
	WINCONSTEXPR DisplayableBase(const char& display) : DisplayableBase(display, color::setcolor::white) {}

	friend std::ostream& operator<<(std::ostream& os, const DisplayableBase& obj)
	{
		return os << obj.color << obj.display << color::reset;
	}
};

struct Positionable {
	point pos;

	Positionable() = default;
	Positionable(point&& p) : pos{ p } {}
	Positionable(const point& p) : pos{ p } {}
	Positionable(const long long& x, const long long& y) : pos{ x, y } {}

	void move(point&& diff)
	{
		pos.x += std::forward<point::first_type>(diff.x);
		pos.y += std::forward<point::second_type>(diff.y);
	}
};



template<var::numeric T>
struct StatBase {
private:
	T current;
	T max;

	void clamp()
	{
		if (current < 0.0f)
			current = 0.0f;
		else if (current > max)
			current = max;
	}

public:
	StatBase(const T& max) : current{ max }, max{ max } {}
	StatBase(const T& max, const T& current) : current{ max }, max{ max } {}

	operator T() const { return current; }

	T getMax() const
	{
		return max;
	}
	T getCurrent() const
	{
		return current;
	}
	void setMax(const T& newMax)
	{
		max = newMax;
		clamp();
	}
	void setCurrent(const T& newCurrent)
	{
		current = newCurrent;
		clamp();
	}

	void increment(const T& amount)
	{
		current += amount;
		clamp();
	}
	void decrement(const T& amount)
	{
		current -= amount;
		clamp();
	}
};
using StatFloat = StatBase<float>;
