#pragma once
#include "../base/BaseAttributes.hpp"

#include <make_exception.hpp>
#include <TermAPI.hpp>

using frame_elem = DisplayableBase;
using frame_container = std::vector<frame_elem>;

inline static constexpr point from1D(const position& index, const position& sizeX)
{
	return{ index / sizeX, index % sizeX };
}
inline static constexpr position to1D(const point& p, const position& sizeX)
{
	return static_cast<position>((static_cast<long long>(sizeX) * p.y) + p.x);
}
inline static constexpr position to1D(const long long& x, const long long& y, const position& sizeX)
{
	return static_cast<position>((static_cast<long long>(sizeX) * y) + x);
}

/// @brief	This is a comparison operator used by framebuffer::display().
inline static bool operator==(const frame_elem& l, const frame_elem& r)
{
	return l.display == r.display && l.color == r.color;
}

struct frame {
	frame_container cont;
	position SizeX, SizeY;
	position Size;

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
			"Invalid frame size \'", cont.size(), "\'!\n",
			indent(10), "X-Axis Size:  ", SizeX, '\n',
			indent(10), "Y-Axis Size:  ", SizeY, '\n',
			indent(10), "Total Size:   ", Size
		);
	}
	WINCONSTEXPR frame(const position& sizeX, const position& sizeY, const frame_container& cont) : cont{ cont }, SizeX{ sizeX }, SizeY{ sizeY }, Size{ sizeX * sizeY }
	{
		if (this->cont.size() != SizeX * SizeY) throw make_exception(
			"Invalid frame size \'", cont.size(), "\'!\n",
			indent(10), "X-Axis Size:  ", SizeX, '\n',
			indent(10), "Y-Axis Size:  ", SizeY, '\n',
			indent(10), "Total Size:   ", Size
		);
	}

	template<std::same_as<frame_elem>... Ts>
	WINCONSTEXPR frame(const position& sizeX, const position& sizeY, Ts&&... elements) : cont{ std::forward<Ts>(elements)... }, SizeX{ sizeX }, SizeY{ sizeY }, Size{ sizeX * sizeY }
	{
		if (this->cont.size() != SizeX * SizeY) throw make_exception(
			"Invalid frame size \'", cont.size(), "\'!\n",
			indent(10), "X-Axis Size:  ", SizeX, '\n',
			indent(10), "Y-Axis Size:  ", SizeY, '\n',
			indent(10), "Total Size:   ", Size
		);
	}

	WINCONSTEXPR frame() : frame(0ull, 0ull) {}

	frame_elem get(const position& x, const position& y) const
	{
		return cont.at(to1D(x, y));
	}
	frame_elem get(const point& pos) const
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

/**
 * @struct	framebuilder
 * @brief	The frame builder is responsible for building static frames.
 */
struct framebuilder {
	framebuilder() = default;
	virtual ~framebuilder() = default;
	/**
	 * @brief			Pure virtual function that retrieves the next frame, given x and y axis constraints.
	 * @param SizeX:	Width of the frame (x-axis).
	 * @param SizeY:	Height of the frame (y-axis).
	 * @returns			frame
	 */
	virtual frame getNext(const position&, const position&) = 0;

	frame getNext(const point& p)
	{
		return getNext(p.x, p.y);
	}
};

/**
 * @struct	framelinker
 * @brief	The frame linker is responsible for communicating between the game's grid positioning system
 *\n		 and the framebuffer in order to display things that aren't static tiles, such as:
 *\n		 - Actors.
 *\n		 - Items.
 *\n		 - Any other temporary gameworld object.
 */
struct framelinker {
	framelinker() = default;
	virtual ~framelinker() = default;

	/**
	 * @brief		Get an override frame element for a specific position.
	 *\n			This pure virtual function must be overloaded by all framelinker derivatives
	 * @param x:	The x-axis position.
	 * @param y:	The y-axis position.
	 * @returns		std::optional<frame_elem>
	 */
	virtual std::optional<frame_elem> get(const position&, const position&) = 0;

	std::optional<frame_elem> get(const point& p)
	{
		return get(p.x, p.y);
	}
};

struct framebuffer {
	position SizeX, SizeY, Size;
	point scbOrigin{ 3, 1 };

private:
	framebuilder* builder{ nullptr };
	framelinker* linker{ nullptr };
public:
	template<std::derived_from<framebuilder> Builder>
	void setBuilder()
	{
		if (builder != nullptr)
			delete builder;
		builder = new Builder();
	}
	template<std::derived_from<framelinker> Linker>
	void setLinker()
	{
		if (linker != nullptr)
			delete linker;
		linker = new Linker();
	}

	framebuffer(const point& size) : SizeX{ size.x }, SizeY{ size.y }, Size{ SizeX * SizeY } {}

	~framebuffer()
	{
		if (builder != nullptr)
			delete builder;
		if (linker != nullptr)
			delete linker;
	}

	point getPointOffset(const position& x_off, const position& y_off, const bool& double_x_axis = true) const
	{
		return{ scbOrigin.x + (double_x_axis ? x_off * 2 : x_off), scbOrigin.y + y_off };
	}

	frame current;

	void initDisplay(frame&& incoming)
	{
		if (linker == nullptr)
			throw make_exception("framebuffer::initDisplay(frame&&) failed:  No frame linker was set!");

		std::cout << term::setScreenBufferSize(SizeX * 2 + scbOrigin.x * 2, SizeY + scbOrigin.y * 2 + 1) << term::CursorVisible(false);

		for (position y{ 0ull }; y < SizeY; ++y) {
			for (position x{ 0ull }; x < SizeX; ++x) {
				const auto& in{ linker->get(x, y).value_or(incoming.get(x, y)) };
				std::cout << term::setCursorPosition(getPointOffset(x, y)) << in.color << in.display << color::reset;
			}
		}
		current = std::move(incoming);
	}
	void initDisplay()
	{
		if (builder == nullptr)
			throw make_exception("framebuffer::display() failed:  No frame builder was set!");
		initDisplay(builder->getNext(SizeX, SizeY));
	}
	void display(frame&& incoming)
	{
		if (linker == nullptr)
			throw make_exception("framebuffer::initDisplay(frame&&) failed:  No frame linker was set!");
		for (position y{ 0ull }; y < SizeY; ++y) {
			for (position x{ 0ull }; x < SizeX; ++x) {
				const auto& in{ linker->get(x, y).value_or(incoming.get(x, y)) }, out{ current.get(x, y) };
				if (in != out) {
					std::cout << term::setCursorPosition(getPointOffset(x, y)) << in.color << in.display << color::reset;
				}
			}
		}

		current = std::move(incoming);
	}
	void display() noexcept(false)
	{
		if (builder == nullptr)
			throw make_exception("framebuffer::display() failed:  No frame builder was set!");
		display(std::move(builder->getNext(SizeX, SizeY)));
	}
};
