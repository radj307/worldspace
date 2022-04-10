#pragma once
#include "../display/framebuilder.hpp"
#include "gamespace.hpp"

#include <make_exception.hpp>

#include <typeinfo>

struct framebuilder_gamespace_config {
	bool show_all_walls{ false };
	bool show_border_walls{ true };
};

struct framebuilder_gamespace : framebuilder {
	gamespace& g;
	framebuilder_gamespace_config cfg;

	framebuilder_gamespace(gamespace& game, const framebuilder_gamespace_config& cfg = {}) : g{ game }, cfg{ cfg } {}

	bool showTile(const position& x, const position& y) const
	{
		auto* tile{ g.grid.get(x, y) };

		const bool& check_type{ cfg.show_all_walls || cfg.show_border_walls };

		Player* player{ &g.player };
		const std::vector<point>& nearbyToPlayer{ player->getPos().getAllPointsWithinCircle(player->visRange, g.boundaries, true) };

		const auto& isNearby{ [&nearbyToPlayer](const point& pos) -> bool {
			for (const auto& it : nearbyToPlayer)
				if (it == pos)
					return true;
			return false;
		} };

		if (check_type && typeid(*tile) == typeid(walltile)) {
			if (cfg.show_all_walls)
				return true;
			else if (cfg.show_border_walls && (x == 0 || x == g.grid.SizeX) && (y == 0 || y == g.grid.SizeY))
				return true;
			//return cfg.show_all_walls || (cfg.show_border_walls && (x == 0 || x == g.grid.SizeX) && (y == 0 || y == g.grid.SizeY)) || isNearby({ x, y });
		}
		else return isNearby({ x, y });
	}

	frame getNext(const position& sizeX, const position& sizeY) override
	{
		if (sizeX != g.grid.SizeX || sizeY != g.grid.SizeY) throw make_exception(
			"framebuilder_matrix::getNext() failed:  Requested invalid matrix size!\n",
			indent(10), "Requested Size:  (", sizeX, ", ", sizeY, ")\n",
			indent(10), "Expected Size:   (", g.grid.SizeX, ", ", g.grid.SizeY, ")"
		);

		frame f{ sizeX, sizeY };
		f.reserve(static_cast<size_t>(sizeX * sizeY));

		for (position y{ 0 }; y < sizeY; ++y) {
			for (position x{ 0 }; x < sizeX; ++x) {
				if (showTile(x, y))
					f.emplace_back(g.grid.get(x, y)->operator frame_elem());
				else f.emplace_back(frame_elem{ ' ', color::setcolor::placeholder, false });
			}
		}

		f.shrink_to_fit();
		return f;
	}
};
