#pragma once
#include "display/framelinker.hpp"
#include "gamespace.hpp"
#include "point.h"

struct framelinker_gamespace : framelinker {
	gamespace& g;
	flare* active{ nullptr };

	framelinker_gamespace(gamespace& gamespace) : g { gamespace } {}

	void preFrame() override
	{
		if (active == nullptr && !g.flares.empty())
			active = g.flares.back().get();
	}
	void postFrame() override
	{
		if (active != nullptr) {
			if (active->framesRemaining-- <= 0) {
				active = nullptr;
				g.flares.pop();
			}
		}
	}

	virtual frame_elem& link(frame_elem& e, const position& x, const position& y) override
	{
		if (!e.enableLinking)
			return e;

		if (active != nullptr) // flare
			if (const auto& color{ active->getFlareAt(x, y) }; color.has_value())
				e += color.value();

		if (auto* actor{ g.getActorAt(x, y) }; actor != nullptr)
			e += *actor;
		else if (auto* proj{ g.getProjectileAt(x, y) }; proj != nullptr)
			e += *proj;
		return e;
	}
};
