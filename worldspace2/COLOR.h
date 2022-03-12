#pragma once
#include <palette.hpp>

#include <utility>

enum class COLOR : unsigned char {
	WHITE,
	BLACK,
	RED,
	GREEN,
	BLUE,
};

static color::palette<COLOR> palette{
	std::make_pair(COLOR::WHITE, color::white),
	std::make_pair(COLOR::BLACK, color::black),
	std::make_pair(COLOR::RED, color::red),
	std::make_pair(COLOR::GREEN, color::green),
	std::make_pair(COLOR::BLUE, color::blue),
};

