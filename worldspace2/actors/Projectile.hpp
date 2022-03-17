#pragma once
#include "../base/BaseAttributes.hpp"
#include "../world/point.h"
#include "ActorBase.hpp"

#include <optional>

struct ProjectileTemplate {
protected:
	std::optional<DisplayableBase> displayableBase;
	std::optional<StatFloat> damage;
	std::optional<bool> piercing;

public:
	inline static const DisplayableBase
		default_displayableBase{ '*', color::setcolor{ color::rgb_to_sgr(1.0, 0.5, 0.0) } };
	inline static const StatFloat
		default_damage{ 20.0f };
	inline static const bool
		default_piercing{ true };

	ProjectileTemplate(
		const std::optional<DisplayableBase>& displayableBase = std::nullopt,
		const std::optional<StatFloat>& damage = std::nullopt,
		const std::optional<bool>& piercing = std::nullopt
	) :
		displayableBase{ displayableBase },
		damage{ damage },
		piercing{ piercing }
	{
	}

	DisplayableBase getDisplayableBase() const
	{
		return displayableBase.value_or(default_displayableBase);
	}
	StatFloat getDamage() const
	{
		return damage.value_or(default_damage);
	}
	bool getPiercing() const
	{
		return piercing.value_or(default_piercing);
	}
};

struct Projectile : DisplayableBase, Positionable {
protected:
	ActorBase* owner{ nullptr };
	unsigned distance{ 0u };

public:
	point direction;
	StatFloat damage;
	bool piercing;

	Projectile(ActorBase* owner, const point& origin, const point& direction, const ProjectileTemplate& t) :
		DisplayableBase(t.getDisplayableBase()),
		Positionable(origin),
		owner{ owner },
		direction{ direction },
		damage{ t.getDamage() },
		piercing{ t.getPiercing() }
	{}

	Projectile(ActorBase* owner, const point& origin, const point& direction, const float& damage, const bool& piercing = true, const char& display = '*', const color::setcolor& color = color::setcolor{ color::rgb_to_sgr(1.0, 0.5, 0.0) }) :
		DisplayableBase(display, color),
		Positionable(origin),
		owner{ owner },
		direction{ direction },
		damage{ damage },
		piercing{ piercing }
	{}

	ActorBase* getOwner()
	{
		return owner;
	}

	void move()
	{
		++distance;
		pos += direction;
	}

	unsigned getDistanceTravelled() const
	{
		return distance;
	}

	point nextPos() const
	{
		return pos + direction;
	}
};
