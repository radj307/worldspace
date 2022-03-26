#pragma once
#include "../base/BaseAttributes.hpp"

#include <optional>

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
	 * @brief	This is an optional function that is called before each display cycle.
	 */
	virtual void preFrame() {}
	/**
	 * @brief	This is an optional function that is called after each display cycle.
	 */
	virtual void postFrame() {}

	/**
	 * @brief		Get an override frame element for a specific position.
	 *\n			This pure virtual function must be overloaded by all framelinker derivatives
	 * @param x:	The x-axis position.
	 * @param y:	The y-axis position.
	 * @returns		std::optional<frame_elem>
	 */
	virtual std::optional<DisplayableBase> get(const position&, const position&) = 0;

	std::optional<DisplayableBase> get(const point& p)
	{
		return get(p.x, p.y);
	}
};