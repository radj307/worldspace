#pragma once
#include "../world/point.h"

#include <TermAPI.hpp>
#include <var.hpp>

#include <optional>

struct DisplayableBase {
	char display;
	color::setcolor color;

	WINCONSTEXPR DisplayableBase(const char& display, const color::setcolor& color) : display{ display }, color{ color } {}
	WINCONSTEXPR DisplayableBase(const char& display) : DisplayableBase(display, color::setcolor::white) {}

	bool operator==(const DisplayableBase& o) const { return display == o.display && color == o.color; }
	bool operator!=(const DisplayableBase& o) const { return display != o.display || color != o.color; }

	friend std::ostream& operator<<(std::ostream& os, const DisplayableBase& obj)
	{
		return os << obj.color << obj.display << color::reset;
	}
};

/**
 * @struct	Positioned
 * @brief	Immutable position attribute.
 */
struct Positioned {
private:
	point pos;
public:
	Positioned() = default;
	Positioned(point&& p) : pos{ std::move(p) } {}
	Positioned(const point& p) : pos{ p } {}
	Positioned(const position& x, const position& y) : pos{ x, y } {}

	point getPos() const
	{
		return pos;
	}
	void setPos(point&& p)
	{
		pos = std::move(p);
	}
	void setPos(const point& p)
	{
		pos = p;
	}
};

/**
 * @struct	Positionable
 * @brief	Mutable position attribute.
 */
struct Positionable {
private:
	point pos;
public:
	Positionable() = default;
	Positionable(point&& p) : pos{ std::move(p) } {}
	Positionable(const point& p) : pos{ p } {}
	Positionable(const position& x, const position& y) : pos{ x, y } {}

	void movePosBy(point&& diff)
	{
		pos += std::forward<point>(diff);
	}
	void movePosBy(const point& diff)
	{
		pos += diff;
	}
	void moveUp(const unsigned& count = 1u)
	{
		movePosBy({ 0, -static_cast<position>(count) });
	}
	void moveDown(const unsigned& count = 1u)
	{
		movePosBy({ 0, static_cast<position>(count) });
	}
	void moveLeft(const unsigned& count = 1u)
	{
		return movePosBy({ -static_cast<position>(count), 0ll });
	}
	void moveRight(const unsigned& count = 1u)
	{
		movePosBy({ static_cast<position>(count), 0 });
	}

	template<class Pred>
	bool tryMovePosBy(point&& diff, const Pred& pred)
	{
		const auto& p{ pos + std::forward<point>(diff) };
		if (pred(p)) {
			pos = p;
			return true;
		}
		return false;
	}
	template<class Pred>
	bool tryMoveUp(const Pred& pred, const unsigned& count = 1u)
	{
		return tryMovePosBy({ 0, -static_cast<position>(count) }, pred);
	}
	template<class Pred>
	bool tryMoveDown(const Pred& pred, const unsigned& count = 1u)
	{
		return tryMovePosBy({ 0, static_cast<position>(count) }, pred);
	}
	template<class Pred>
	bool tryMoveLeft(const Pred& pred, const unsigned& count = 1u)
	{
		return tryMovePosBy({ -static_cast<position>(count), 0ll }, pred);
	}
	template<class Pred>
	bool tryMoveRight(const Pred& pred, const unsigned& count = 1u)
	{
		return tryMovePosBy({ static_cast<position>(count), 0 }, pred);
	}

	void setPos(const point& p)
	{
		pos = p;
	}
	point getPos() const
	{
		return pos;
	}
};

template<var::numeric T>//, T MIN_VALUE = static_cast<T>(0)>
struct StatBaseNoMax {
	inline static constexpr const T MIN{ static_cast<T>(0.0f) }, ZERO{ static_cast<T>(0) };
protected:
	T current;	///< @brief	The current value of this Stat.
	T modifier;	///< @brief	The current modifier value of this Stat. This is added to the current value when using it for calculations.

	/**
	 * @brief	Clamp the current value using (MIN).
	 *\n		This function may be overridden.
	 */
	virtual void clamp()
	{
		if (current < MIN)
			current = MIN;
	}
	/**
	 * @brief		Clamp a given value within a given range from (min) to (max).
	 * @param min	Minimum allowable value.
	 * @param v		Input value to clamp between min & max.
	 * @param max	Optional maximum allowable value.
	 * @returns		T
	 */
	static T clamp(const T& min, const T& v, const std::optional<T>& max = std::nullopt)
	{
		if (v < min)
			return min;
		else if (max.has_value())
			if (const auto& maxValue{ max.value() }; v > maxValue)
				return maxValue;
		return v;
	}

public:
	StatBaseNoMax(T&& v, T&& mod = ZERO) : current{ std::move(v) }, modifier{ std::move(mod) } {}
	StatBaseNoMax(const T& v, const T& mod = ZERO) : current{ v }, modifier{ mod } {}

	T getCurrent() const { return current; }
	T getModifier() const { return modifier; }
	void setCurrent(const T& value) { current = value; }
	void setModifier(const T& value) { modifier = value; }

	operator T() const
	{
		return current + modifier;
	}

	auto operator-(const T& amount) const
	{
		return StatBaseNoMax<T>{ current - amount, modifier };
	}
	auto operator-(const StatBaseNoMax<T>& o) const
	{
		return StatBaseNoMax<T>{ current - o.current, modifier };
	}
	auto operator-=(const T& amount)
	{
		current -= amount;
		clamp();
		return *this;
	}

	auto operator+(const T& amount) const
	{
		return StatBaseNoMax<T>{ current + amount, modifier };
	}
	auto operator+(const StatBaseNoMax<T>& o) const
	{
		return StatBaseNoMax<T>{ current + o.current, modifier };
	}
	auto operator+=(const T& amount)
	{
		current += amount;
		clamp();
		return *this;
	}

	virtual void increment(const T& amount)
	{
		current += amount;
		clamp();
	}
	virtual void decrement(const T& amount)
	{
		current -= amount;
		clamp();
	}
};


template<var::numeric T>//, T MIN_VALUE = static_cast<T>(0)>
struct StatBase : public StatBaseNoMax<T> {//, MIN_VALUE> {
protected:
	T max;

	void clamp() override
	{
		if (this->current < this->MIN)
			this->current = this->MIN;
		else if (this->current > max)
			this->current = max;
		if (max < this->MIN)
			max = this->MIN;
	}

public:
	StatBase(const T& max) : StatBaseNoMax<T>(max), max{ max } {}
	StatBase(const T& max, const T& current, const T& modifier = StatBaseNoMax<T>::ZERO) : StatBaseNoMax<T>(max, modifier), max{ max } {}

	operator T() const { return this->current + this->modifier; }

	T getMax() const
	{
		return max;
	}
	void setMax(const T& newMax)
	{
		max = newMax;
		clamp();
	}

	/**
	 * @brief		Returns the current stat as an unsigned integer out of the given integral scale number.
	 *\n			This function uses floating-point math, then rounds the result to the nearest integer.
	 * @param scale	A power of 10 to multiply the result of (current / max) by.
	 *\n			For example, if (max = 100, current = 50, scale = 10) then (result = 5) because (current / max * 10) = 5
	 * @returns		unsigned
	 */
	int toScale(const int& scale = 10) const
	{
		return static_cast<int>(std::round(this->current / max * static_cast<T>(scale)));
	}
	/**
	 * @brief		Returns the current stat as an unsigned integer out of the given integral scale number.
	 *\n			This function variant includes the current modifier value in the output.
	 *\n			This function uses floating-point math, then rounds the result to the nearest integer.
	 * @param scale	A power of 10 to multiply the result of (current / max) by.
	 *\n			For example, if (max = 100, current = 50, scale = 10) then (result = 5) because (current / max * 10) = 5
	 * @returns		unsigned
	 */
	int toScaleWithModifier(const int& scale = 10) const
	{
		return static_cast<int>(std::round((this->current + this->modifier) / max * static_cast<T>(scale)));
	}

	/**
	 * @brief	Get the current value of this stat, as a percentage out of 100.
	 * @returns	int
	 *\n		Range: 0 - 99
	 */
	int asPercentage(const bool& includeMod = true) const
	{
		return includeMod ? toScaleWithModifier(100) : toScale(100);
	}
};

using StatFloat = StatBase<float>;
using StatInt = StatBase<int>;
using StatUnsigned = StatBase<unsigned>;


struct TargetStat {
private:
	unsigned char _value;
public:
	template<typename T>
	constexpr TargetStat(T&& value) : _value{ static_cast<unsigned char>(value) } {}
	constexpr TargetStat(const TargetStat& o) : _value{ o._value } {}
	constexpr TargetStat(TargetStat&& o) noexcept : _value{ std::move(o._value) } {}

	TargetStat& operator=(TargetStat&& o) noexcept
	{
		_value = std::move(o._value);
		return *this;
	}
	TargetStat& operator=(const TargetStat& o)
	{
		_value = o._value;
		return *this;
	}

	TargetStat& operator|=(const int& o)
	{
		_value |= o;
		return *this;
	}
	TargetStat& operator^=(const int& o)
	{
		_value ^= o;
		return *this;
	}
	TargetStat& operator&=(const int& o)
	{
		_value &= o;
		return *this;
	}

	operator int() const { return static_cast<int>(_value); }

	template<std::same_as<TargetStat>... Ts>
	bool contains(Ts&&... stats) const
	{
		return var::variadic_or(((_value & std::forward<Ts>(stats)._value) != 0)...);
	}

	static const TargetStat NULL_STAT, HEALTH, STAMINA, DAMAGE, DEFENSE, FEAR, AGGRESSION;
};

inline const constexpr TargetStat TargetStat::NULL_STAT{ 0 };
inline const constexpr TargetStat TargetStat::HEALTH{ 1 };
inline const constexpr TargetStat TargetStat::STAMINA{ 2 };
inline const constexpr TargetStat TargetStat::DAMAGE{ 4 };
inline const constexpr TargetStat TargetStat::DEFENSE{ 8 };
inline const constexpr TargetStat TargetStat::FEAR{ 16 };
inline const constexpr TargetStat TargetStat::AGGRESSION{ 32 };
