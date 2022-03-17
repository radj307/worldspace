#pragma once
#include "../display/framelinker.hpp"

struct framelinker_gamespace : framelinker {
	gamespace& g;
	framelinker_gamespace(gamespace& gamespace) : g{ gamespace } {}

	std::optional<DisplayableBase> get(const position& x, const position& y) override
	{
		if (auto* actor{ g.getActorAt(x, y) }; actor != nullptr && !actor->isDead())
			return *actor;
		else if (auto* proj{ g.getProjectileAt(x, y) }; proj != nullptr)
			return *proj;
		return std::nullopt;
	}
};
