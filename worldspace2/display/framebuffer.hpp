#pragma once
#include "../base/BaseAttributes.hpp"
#include "statpanel.hpp"
#include "frame.hpp"
#include "framebuilder.hpp"
#include "framelinker.hpp"

#include <make_exception.hpp>
#include <TermAPI.hpp>

struct framebuffer {
	// @brief	The size of the grid on the x-axis (horizontal).
	position SizeX;
	// @brief	The size of the grid on the y-axis (vertical).
	position SizeY;
	// @brief	The total size of the grid, in cells. (SizeX * SizeY)
	position Size;

	// @brief	The origin point (top-left corner) of the displayed frame.
	point csbOrigin{ 3, 1 };

	// @brief	The last frame to be fully printed. This is used as a comparison for selectively-updating cells.
	frame current;

	// @brief	When true, the display(frame) function won't do anything. This is never set internally.
	bool freeze{ false };

private:
	bool initialized{ false };

	framebuilder* builder{ nullptr };
	framelinker* linker{ nullptr };
	statpanel* panel{ nullptr };
public:
	/**
	 * @brief			Set the framebuilder type to use.
	 *\n				Creates a new framebuilder pointer on the heap, which is cleaned up in the framebuffer destructor.
	 * @tparam Builder	A type derived from framebuilder.
	 */
	template<std::derived_from<framebuilder> Builder, typename... Args>
	void setBuilder(Args&&... args)
	{
		if (builder != nullptr)
			delete builder;
		builder = new Builder(std::forward<Args>(args)...);
	}
	/**
	 * @brief			Set the framelinker type to use.
	 *\n				Creates a new framelinker pointer on the heap, which is cleaned up in the framebuffer destructor.
	 * @tparam Linker	A type derived from framelinker.
	 */
	template<std::derived_from<framelinker> Linker, typename... Args>
	void setLinker(Args&&... args)
	{
		if (linker != nullptr)
			delete linker;
		linker = new Linker(std::forward<Args>(args)...);
	}
	template<std::derived_from<statpanel> Panel>
	void setPanel(ActorBase* bindTarget)
	{
		panel = new statpanel(csbOrigin.y + SizeY + STATPANEL_PADDING, bindTarget);
	}

	framebuffer(const point& size, const point& csbOrigin = { 3, 1 }) : SizeX{ size.x }, SizeY{ size.y }, Size{ SizeX * SizeY }, csbOrigin{ csbOrigin }, current{ SizeX, SizeY } {}
	framebuffer(const position& sizeX, const position& sizeY) : SizeX{ sizeX }, SizeY{ sizeY }, Size{ SizeX * SizeY }, current{ SizeX, SizeY } {}

	~framebuffer() noexcept
	{
		// delete pointers
		if (builder != nullptr)
			delete builder;
		if (linker != nullptr)
			delete linker;
		if (panel != nullptr)
			delete panel;
	}

	bool isInitialized() const
	{
		return initialized;
	}

	/**
	 * @brief					Resolve a point on the grid to screen buffer coordinates.
	 *\n						This function is designed to be passed directly to term::setCursorPosition().
	 * @param x_off				X Offset. (Grid Position)
	 * @param y_off				Y Offset. (Grid Position)
	 * @param double_x_axis		When true, multiplies the size of the x-axis by 2 to accomodate spaces between columns.
	 * @returns					std::pair<unsigned, unsigned>
	 *							 1. X-Axis (Horizontal / Column Index) Position.
	 *							 2. Y-Axis (Vertical / Row Index) Position.
	 */
	std::pair<unsigned, unsigned> getPointOffset(const position& x_off, const position& y_off, const bool& double_x_axis = true) const
	{
		return{ static_cast<unsigned>(csbOrigin.x + (double_x_axis ? x_off * 2 : x_off)), static_cast<unsigned>(csbOrigin.y + y_off) };
	}

	/**
	 * @brief			Initializes the screen buffer size, hides the cursor, and prints the first frame.
	 * @param incoming	First frame instance.
	 */
	void initDisplay(frame&& incoming)
	{
		deinitDisplay();

		if (linker == nullptr)
			throw make_exception("framebuffer::initDisplay(frame&&) failed:  No frame linker was set!");

		std::cout << term::CursorVisible(false);

		std::cout << term::setScreenBufferSize((SizeX * 2 + csbOrigin.x * 2), (SizeY + (csbOrigin.y * 2) + STATPANEL_HEIGHT + (STATPANEL_PADDING * 2 - 1)));

		for (position y{ 0ull }; y < SizeY; ++y) {
			for (position x{ 0ull }; x < SizeX; ++x) {
				const auto& linked{ linker->get(x, y) };
				const auto& in{ incoming.get(x, y) };
				std::cout << term::setCursorPosition(getPointOffset(x, y));
				if (linked.has_value())
					std::cout << linked.value();
				else
					std::cout << in;
				std::cout << color::reset;
			}
		}

		current = std::move(incoming);

		// statpanel
		if (panel != nullptr)
			panel->display();

		initialized = true;
	}
	/**
	 * @brief	Initializes the screen buffer size, hides the cursor, and prints the first frame received from the framebuilder.
	 */
	void initDisplay()
	{
		if (builder == nullptr)
			throw make_exception("framebuffer::display() failed:  No frame builder was set!");
		initDisplay(builder->getNext(SizeX, SizeY));
	}
	/**
	 * @brief			Displays a frame by comparing it to the previous frame, and only updating the changed cells.
	 * @param incoming	The current frame to display.
	 */
	void display(frame incoming)
	{
		if (freeze)
			return;

		if (linker == nullptr)
			throw make_exception("framebuffer::initDisplay(frame&&) failed:  No frame linker was set!");
		else if (!initialized) {
			initDisplay();
			return;
		}

		for (position y{ 0ull }; y < SizeY; ++y) {
			for (position x{ 0ull }; x < SizeX; ++x) {
				const auto& out{ current.getRef(x, y) }; // outgoing frame pos
				auto& in{ incoming.getRef(x, y) }; // incoming frame pos
				if (const auto& linked{ linker->get(x, y) }; linked.has_value())
					in.emplace_back(linked.value()); // insert linker output
				if (in != out) // display:
					std::cout << term::setCursorPosition(getPointOffset(x, y)) << in << color::reset;
			}
		}

		current = std::move(incoming);

		if (panel != nullptr)
			panel->display();
	}
	/**
	 * @brief	Displays a frame by comparing it to the previous frame, and only updating the changed cells. Uses the framebuilder to create the frame.
	 */
	void display() noexcept(false)
	{
		if (builder == nullptr)
			throw make_exception("framebuffer::display() failed:  No frame builder was set!");
		display(std::move(builder->getNext(SizeX, SizeY)));
	}

	void deinitDisplay()
	{
		std::cout << term::clear;
		initialized = false;
	}
};
