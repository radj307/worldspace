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
	 * @brief		Link the specified frame.
	 * @param e		A reference to the frame element to be linked.
	 * @param x		The x-axis position of the frame element.
	 * @param y		The y-axis position of the frame element.
	 * @returns		frame_elem&
	 */
	virtual frame_elem& link(frame_elem&, const position&, const position&) = 0;
};