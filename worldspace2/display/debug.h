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

struct framelinker_debug : framelinker {
	std::optional<frame_elem> get(const position& x, const position& y) override
	{
		if (RNG.get(0.0, 100.0) < 1.0)
			return frame_elem{ '?', color::setcolor::red };
		return std::nullopt;
	}
};