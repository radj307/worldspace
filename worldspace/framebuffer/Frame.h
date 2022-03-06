/**
 * @file Frame.h
 * @author radj307
 * @brief Contains the Frame object, which represents a single frame drawn to the console window. \n
 * Used in FrameBuffer.h
 */
#pragma once
#include <vector>

#include "../gameworld/Coord.h"

 /**
  * @struct Frame
  * @brief Represents a single frame shown in the console.
  * Used for output buffering to only change characters that have been modified between frames.
  */
struct Frame {
	// This frame's buffer
	std::vector<std::vector<char> > _frame;
	Coord _origin;
	bool _space_columns;

	/** CONSTRUCTOR
	 * Frame()
	 * @brief Instantiate a blank frame.
	 */
	Frame() : _origin({ 0, 0 }), _space_columns(false)
	{
	}

	/** CONSTRUCTOR
	 * Frame(vector<vector<char>>)
	 * @brief Instantiate a pre-made frame
	 */
	explicit Frame(std::vector<std::vector<char> > frameMatrix, const Coord& frameOrigin = Coord(0, 0), const bool formatWithSpaces = true) : _frame(std::move(frameMatrix)), _origin(frameOrigin), _space_columns(formatWithSpaces)
	{
	}

	/**
	 * isValidSize()
	 * @brief Check if all rows in this frame are the same size.
	 * @returns bool	- ( true = frame is valid ) ( false = frame is invalid, row size is variable. )
	 */
	bool isValidSize()
	{
		if (!_frame.empty()) {
			const auto expected_row_length{ _frame.at(0).size() };
			for (auto& it : _frame)
				if (it.size() != expected_row_length)
					return false;
			return true;
		}
		return false;
	}

	/**
	 * size()
	 * @brief Returns the size of the frame if it is valid.
	 * @returns Coord
	 */
	Coord size()
	{
		if (!_frame.empty() && isValidSize())
			return { static_cast<long>(_frame.at(0).size()), static_cast<long>(_frame.size()) };
		return { 0, 0 };
	}

	/**
	 * draw()
	 * @brief Draws this frame to the console at it's origin polong.
	 */
	void draw()
	{
		// use dual-iterators to iterate both the frame, and console position from the origin offset
		for (long consoleY{ _origin._y }, frameY{ 0 }; consoleY < _origin._y + static_cast<long>(_frame.size()); consoleY++, frameY++) {
			for (long consoleX = _origin._x, frameX{ 0 }; consoleX < _origin._x + static_cast<long>(_frame.at(frameY).size()); consoleX++, frameX++) {
				printf("%s%c", term::setCursorPosition(consoleX * 2, consoleY).c_str(), _frame.at(frameY).at(frameX));
				if (_space_columns)
					printf(" ");
			}
		}
	}

	// Stream insertion operator
	friend std::ostream& operator<<(std::ostream& os, const Frame& f)
	{
		for (const auto& y : f._frame) {
			for (auto x : y) {
				os << x << ' ';
			}
			os << std::endl;
		}
		return os;
	}
};
