#pragma once
#include "../base/ActorBase.hpp"

struct tile : DisplayableBase {
	virtual void effect(ActorBase*) const = 0;

	tile(const char& display, const color::setcolor& color) : DisplayableBase(display, color) {}
	virtual ~tile() = default;
};

struct walltile : tile {
	walltile() : tile('#', color::setcolor::white) {}
};
struct floortile : tile {
	floortile() : tile('_', color::setcolor::white) {}
};
struct traptile : tile {
	StatFloat damage;

	void effect(ActorBase* actor) const override
	{

	}

	traptile(const float& damage) : tile('O', color::setcolor::cyan), damage{ damage } {}
};
