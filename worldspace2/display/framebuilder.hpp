#pragma once
#include "frame.hpp"

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
