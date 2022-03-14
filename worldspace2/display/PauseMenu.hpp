/**
 * @file	paused.h
 * @author	radj307
 * @brief	Contains the pause menu's display functions.
 */
#pragma once
#include "positionable_text.h"

struct PauseMenu : positionable_text {
	inline static std::vector<std::string> text{
		"Game Paused"
	};
	color::setcolor color;

	PauseMenu(const point& csbTopMiddle, const color::setcolor& color) : positionable_text(csbTopMiddle, text), color{ color } {}

	void display()
	{
		std::cout << color << *this << color::reset;
	}
};
