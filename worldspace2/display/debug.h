#pragma once
#include "frame.hpp"

#include <xRand.hpp>

rng::tRand RNG;

struct framebuilder_debug : framebuilder {
	frame_elem frameElem{ '_', color::setcolor::white };
	frame getNext(const position& SizeX, const position& SizeY) override
	{
		frame f{ SizeX, SizeY };

		for (position i{ 0ull }, end{ SizeX * SizeY }; i < end; ++i)
			f.cont.emplace_back(frameElem);

		return f;
	}
};

point displayPos{};

struct framelinker_debug : framelinker {
	std::optional<DisplayableBase> get(const position& x, const position& y) override
	{
		if (x == displayPos.x && y == displayPos.y)
			return DisplayableBase{ '?', color::setcolor::green };
		else if (RNG.get(0.0, 100.0) < 1.0)
			return DisplayableBase{ '?', color::setcolor::red };
		return std::nullopt;
	}
};