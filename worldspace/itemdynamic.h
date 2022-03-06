/**
 * @file itemdynamic.h
 * @author radj307
 * @brief Contains dynamic items that can be picked up by actors, and used later.
 */
#pragma once
#include <utility>

#include "actorbase.h"
#include "itemstats.h"

struct ItemDynamicBase : ItemStats { protected:
	std::unique_ptr<ActorBase> _owner { nullptr };
	
	virtual void func() = 0;

	virtual bool cond() = 0;

public:
	ItemDynamicBase(const char display, const color::setcolor& displayColor, std::string name, const int max_uses) : ItemStats{ display, displayColor, std::move(name), max_uses } {}
	
	ItemDynamicBase(ActorBase* owner, const char display, const color::setcolor& displayColor, std::string name, const int max_uses) : ItemStats{ display, displayColor, std::move(name), max_uses }, _owner{ owner } {}
	
	bool attempt_use()
	{
		if ( _owner != nullptr && cond() ) {
			func();
			--_use_count;
			return true;
		}
		return false;
	}
};

struct ItemDynamicHealth : ItemDynamicBase { private:
	int _amount_restored;

	void func() override
	{
		_owner->modHealth(_amount_restored);
	}

	bool cond() override { return _owner != nullptr && _owner->getHealth() < _owner->getMaxHealth(); }

public:
	explicit ItemDynamicHealth(const int amountRestored) : ItemDynamicBase{ '*', color::setcolor(color::red, color::Layer::B), "Health Potion", 1 }, _amount_restored{ amountRestored } {}
};

struct ItemDynamicStamina : ItemDynamicBase { private:
	int _amount_restored;

	void func() override
	{
		_owner->modStamina(_amount_restored);
	}

	bool cond() override { return _owner != nullptr && _owner->getStamina() < _owner->getMaxStamina(); }
	
public:
	explicit ItemDynamicStamina(const int amountRestored) : ItemDynamicBase{ '*', color::setcolor(color::green, color::Layer::B), "Stamina Potion", 1 }, _amount_restored{ amountRestored } {}
};

struct ItemDynamicAoE : ItemStats { protected:
	std::unique_ptr<ActorBase> _owner { nullptr };
	int _range;

	virtual void func(const std::vector<NPC*>&) = 0;

	virtual bool cond() = 0;

public:
	explicit ItemDynamicAoE(const int range, const char display, const color::setcolor& displayColor, std::string name, const int max_uses) : ItemStats{ display, displayColor, std::move(name), max_uses }, _range { range } {}

	bool attempt_use(const std::vector<NPC*>& targets)
	{
		if ( _owner != nullptr && !targets.empty() && cond() ) {
			func(targets);
			--_use_count;
			return true;
		}
		return false;
	}
};

struct ItemDynamicAoESmokeBomb : ItemDynamicAoE { private:

	void func(const std::vector<NPC*>& targets) override
	{
		for ( const auto& it : targets )
			it->removeAggro();
	}

	bool cond() override { return _owner != nullptr; }

public:
	explicit ItemDynamicAoESmokeBomb(const int range) : ItemDynamicAoE{ range, '+', color::setcolor(color::white, color::Layer::B), "Smoke Bomb", 1 } {}

	[[nodiscard]] int getRange() const { return _range; }
};