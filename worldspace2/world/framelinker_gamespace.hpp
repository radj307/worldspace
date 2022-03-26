#pragma once
#include "../display/framelinker.hpp"

struct framelinker_gamespace : framelinker {
	gamespace& g;
	flare* active{ nullptr };

	framelinker_gamespace(gamespace& gamespace) : g{ gamespace } {}

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

	std::optional<DisplayableBase> get(const position& x, const position& y) override
	{
		if (auto* actor{ g.getActorAt(x, y) }; actor != nullptr && !actor->isDead())
			return *actor;
		else if (auto* proj{ g.getProjectileAt(x, y) }; proj != nullptr)
			return *proj;
		else if (active != nullptr)
			if (const auto& color{ active->getFlareAt(x, y) }; color.has_value())
				return DisplayableBase{ '\0', color.value() };
		return std::nullopt;
	}
};
