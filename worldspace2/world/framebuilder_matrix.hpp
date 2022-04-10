#pragma once
#include "../display/framebuilder.hpp"
#include "matrix.hpp"

#include <make_exception.hpp>

struct framebuilder_matrix : framebuilder {
	matrix& m;

	framebuilder_matrix(matrix& matrix) : m{ matrix } {}

	frame getNext(const position& sizeX, const position& sizeY) override
	{
		if (sizeX != m.SizeX || sizeY != m.SizeY) throw make_exception(
			"framebuilder_matrix::getNext() failed:  Requested invalid matrix size!\n",
			indent(10), "Requested Size:  (", sizeX, ", ", sizeY, ")\n",
			indent(10), "Expected Size:   (", m.SizeX, ", ", m.SizeY, ")"
		);

		frame f{ sizeX, sizeY };
		f.reserve(static_cast<size_t>(sizeX * sizeY));

		for (position y{ 0 }; y < sizeY; ++y) {
			for (position x{ 0 }; x < sizeX; ++x) {
				f.emplace_back(m.get(x, y)->operator frame_elem());
			}
		}

		f.shrink_to_fit();
		return f;
	}
};
