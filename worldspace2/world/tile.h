#pragma once
#include "../base/BaseAttributes.hpp"
#include "../actors/ActorBase.hpp"
#include "../items/ItemBase.hpp"
#include "../display/frame.hpp"

#include <memory>

struct tile : DisplayableBase {
	/**
	 * @brief				Provides an interface for modifying actors who step on this tile.
	 *\n					This interface is called from gamespace::moveActor().
	 * @param ActorBase*	A pointer to the actor who is currently located on this tile.
	 */
	virtual void effect(ActorBase*) = 0;

	tile(const char& display, const color::setcolor& color) : DisplayableBase(display, color) {}
	tile() : tile('\0', color::setcolor::white) {}

	tile(tile&& o) : tile(std::move(o.display), std::move(o.color)) {}
	tile(const tile& o) : tile(o.display, o.color) {}

	virtual ~tile() = default;

	operator frame_elem() const
	{
		return{ display, color };
	}
};

struct walltile : tile {
	virtual void effect(ActorBase*) override {}

	walltile(const char& display, const color::setcolor& color) : tile(display, color) {}
	walltile() : tile('#', color::setcolor::white) {}
};
struct floortile : tile {
	virtual void effect(ActorBase*) override {}

	floortile(const char& display, const color::setcolor& color) : tile(display, color) {}
	floortile() : tile('_', color::setcolor::white) {}
};
struct traptile : tile {
	StatFloat damage;
	bool armorPiercing;

	virtual void effect(ActorBase* actor) override
	{
		actor->applyDamage(this->damage, this->armorPiercing);
	}

	traptile(const float& damage, const bool& armorPiercing) : tile('O', color::setcolor::cyan), damage{ damage }, armorPiercing{ armorPiercing } {}
	traptile() : traptile(0.0f, false) {}
};
struct doortile : tile {
	virtual void effect(ActorBase* actor) override
	{
		// TODO: Add level change handling (?)
	}

	// TODO: Add parameters & members for level change handling (?)
	doortile() : tile('\xa7', color::setcolor{ ANSI::make_sequence(color::setcolor::black, color::setcolor{ color::white, color::Layer::B }) }) {}
};
struct containertile : tile {
	std::vector<ItemTemplate<float>> items;

	virtual void effect(ActorBase* actor) override
	{
	}

	containertile(std::vector<ItemTemplate<float>>&& items) : tile('\xa4', color::setcolor{ color::green, color::Layer::B }), items{ std::move(items) } {}
};

