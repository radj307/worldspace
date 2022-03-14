#pragma once
#include "../base/BaseAttributes.hpp"

#include <setcolor.hpp>
#include <make_exception.hpp>
#include <indentor.hpp>

#include <vector>

/**
 * @struct	frame_elem
 * @brief	Represents a single cell in a displayed frame.
 *\n		The first element in the container is considered the "base" cell,
 *\n		while the last element is considered the "winning override" cell.
 */
struct frame_elem : std::vector<DisplayableBase> {
	/**
	 * @brief			Constructor
	 * @param display	The character to display for this cell.
	 * @param color		The color to use when printing the display char.
	 */
	frame_elem(const char& display, const color::setcolor& color = color::setcolor::placeholder)
	{
		this->emplace_back(DisplayableBase{ display, color });
	}

	/**
	 * @brief	Retrieve the base displayable cell.
	 * @returns	DisplayableBase
	 * @throws	ex::except
	 *			frame_elem::base() failed:  Cannot retrieve a displayable object from an empty cell!
	 */
	DisplayableBase base() const noexcept(false)
	{
		if (!empty())
			return front();
		throw make_exception("frame_elem::base() failed:  Cannot retrieve a displayable object from an empty cell!");
	}
	/**
	 * @brief	Retrieve the winning override displayable cell.
	 *\n		This function may return the base cell if no overrides exist.
	 * @returns	DisplayableBase
	 * @throws	ex::except : frame_elem::base() failed:  Cannot retrieve a displayable object from an empty cell!
	 */
	DisplayableBase over() const noexcept(false)
	{
		if (!empty())
			return back();
		throw make_exception("frame_elem::over() failed:  Cannot retrieve a displayable object from an empty cell!");
	}

	friend std::ostream& operator<<(std::ostream& os, const frame_elem& e)
	{
		if (!e.empty())
			os << e.back();
		return os;
	}

	/// @brief	This is a comparison operator used by framebuffer::display().
	inline bool operator==(const frame_elem& r)
	{
		if (size() != r.size())
			return false;
		for (size_t i{ 0ull }, end{ size() }; i < end; ++i)
			if (const auto& lv{ at(i) }, & rv{ r.at(i) }; lv != rv)
				return false;
		return true;
	}
};

using frame_container = std::vector<frame_elem>;

/**
 * @brief			Convert a 1-dimensional matrix index to a 2-dimensional point.
 * @param index :	The 1-dimensional index to convert.
 * @param sizeX	:	The x-axis (horizontal) size of the grid.
 * @returns			point
 */
inline static constexpr point from1D(const position& index, const position& sizeX)
{
	return{ index / sizeX, index % sizeX };
}
/**
 * @brief			Convert a 2-dimensional matrix point to a 1-dimensional index.
 * @param p :		A 2-dimensional point in the grid to convert.
 * @param sizeX	:	The x-axis (horizontal) size of the grid.
 * @returns			position
 */
inline static constexpr position to1D(const point& p, const position& sizeX)
{
	return static_cast<position>((static_cast<long long>(sizeX) * p.y) + p.x);
}
/**
 * @brief			Convert a 2-dimensional matrix point to a 1-dimensional index.
 * @param x :		The x-axis of the point to convert.
 * @param y :		The y-axis of the point to convert.
 * @param sizeX	:	The x-axis (horizontal) size of the grid.
 * @returns			position
 */
inline static constexpr position to1D(const long long& x, const long long& y, const position& sizeX)
{
	return static_cast<position>((static_cast<long long>(sizeX) * y) + x);
}

/**
 * @struct	frame
 * @brief	Represents a grid of displayable cells, which is displayed once, then discarded.
 */
struct frame {
private:
	frame_container cont;
	position SizeX, SizeY, Size;

public:
	constexpr position to1D(const point& pos) const
	{
		return ::to1D(pos, SizeX);
	}
	constexpr position to1D(const long long& x, const long long& y) const
	{
		return ::to1D(x, y, SizeX);
	}
	constexpr point from1D(const position& index) const
	{
		return ::from1D(index, SizeX);
	}

	bool validate()
	{
		return cont.size() == Size;
	}

	WINCONSTEXPR frame(const position& sizeX, const position& sizeY) : SizeX{ sizeX }, SizeY{ sizeY }, Size{ sizeX * sizeY } {}
	WINCONSTEXPR frame(const position& sizeX, const position& sizeY, frame_container&& cont) : cont{ std::move(cont) }, SizeX{ sizeX }, SizeY{ sizeY }, Size{ sizeX * sizeY }
	{
		if (this->cont.size() != SizeX * SizeY) throw make_exception(
			"Invalid frame size \'", this->cont.size(), "\'!\n",
			indent(10), "X-Axis Size:  ", SizeX, '\n',
			indent(10), "Y-Axis Size:  ", SizeY, '\n',
			indent(10), "Total Size:   ", Size
		);
	}
	WINCONSTEXPR frame(const position& sizeX, const position& sizeY, const frame_container& cont) : cont{ cont }, SizeX{ sizeX }, SizeY{ sizeY }, Size{ sizeX * sizeY }
	{
		if (this->cont.size() != SizeX * SizeY) throw make_exception(
			"Invalid frame size \'", this->cont.size(), "\'!\n",
			indent(10), "X-Axis Size:  ", SizeX, '\n',
			indent(10), "Y-Axis Size:  ", SizeY, '\n',
			indent(10), "Total Size:   ", Size
		);
	}

	template<std::same_as<frame_elem>... Ts>
	WINCONSTEXPR frame(const position& sizeX, const position& sizeY, Ts&&... elements) : cont{ std::forward<Ts>(elements)... }, SizeX{ sizeX }, SizeY{ sizeY }, Size{ sizeX * sizeY }
	{
		if (this->cont.size() != SizeX * SizeY) throw make_exception(
			"Invalid frame size \'", this->cont.size(), "\'!\n",
			indent(10), "X-Axis Size:  ", SizeX, '\n',
			indent(10), "Y-Axis Size:  ", SizeY, '\n',
			indent(10), "Total Size:   ", Size
		);
	}

	WINCONSTEXPR auto empty() const noexcept { return cont.empty(); }
	WINCONSTEXPR auto size() const noexcept { return cont.size(); }
	WINCONSTEXPR void reserve(const size_t& size)
	{
		cont.reserve(size);
	}
	WINCONSTEXPR void shrink_to_fit()
	{
		cont.shrink_to_fit();
	}

	frame_elem operator[](const size_t& index) const noexcept(false)
	{
		return cont.at(index);
	}

	auto emplace_back(auto&& elem)
	{
		return cont.emplace_back(std::forward<decltype(elem)>(elem));
	}

	frame_elem get(const position& x, const position& y) const
	{
		return cont.at(to1D(x, y));
	}
	frame_elem get(const point& pos) const
	{
		return cont.at(to1D(pos));
	}

	frame_elem& getRef(const position& x, const position& y)
	{
		return cont.at(to1D(x, y));
	}
	frame_elem& getRef(const point& pos)
	{
		return cont.at(to1D(pos));
	}

	void set(const position& x, const position& y, const frame_elem& val)
	{
		cont.at(to1D(x, y)) = val;
	}
	void set(const point& pos, const frame_elem& val)
	{
		cont.at(to1D(pos)) = val;
	}
};
