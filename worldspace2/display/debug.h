#pragma once
#include "framebuilder.hpp"
#include "framelinker.hpp"

#include <xRand.hpp>

struct framebuilder_debug : framebuilder {
	frame_elem frameElem{ '_', color::setcolor::white };
	frame getNext(const position& SizeX, const position& SizeY) override
	{
		frame f{ SizeX, SizeY };

		for (position i{ 0ull }, end{ SizeX * SizeY }; i < end; ++i)
			f.emplace_back(frameElem);

		return f;
	}
};

// @brief	Debugging tool that is used by the framelinker_debug object to simulate a moving actor.
point displayPos{};

struct framelinker_debug : framelinker {
	rng::tRand RNG;
	bool show_noise{false};
	std::optional<DisplayableBase> get(const position& x, const position& y) override
	{
		if (x == displayPos.x && y == displayPos.y)
			return DisplayableBase{ '?', color::setcolor::green };
		else if (show_noise && RNG.get(0.0, 100.0) < 1.0)
			return DisplayableBase{ '?', color::setcolor::red };
		return std::nullopt;
	}
};