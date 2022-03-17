#pragma once
#include "../base/BaseAttributes.hpp"
#include "../world/point.h"
#include "../actors/Faction.hpp"

#include <typeinfo>
#include <optional>
#include <string>

struct ActorTemplate {
private:
	std::optional<DisplayableBase> displayable;
	std::optional<ID> factionID;
	std::optional<unsigned> level;
	std::optional<std::string> name;
	std::optional<StatFloat> health, stamina, damage, defense, fear, aggression;
	std::optional<StatUnsigned> aggressionRange;
	//std::optional<std::vector<std::unique_ptr<ItemBase<float>>>> items;
public:
	inline static const DisplayableBase
		default_displayable{ '\0', color::setcolor{ ""s } };
	inline static const int
		default_factionID{ NULL_FACTION_ID };
	inline static const unsigned
		default_level{ 1u };
	inline static const std::string
		default_name{ "actor template" };
	inline static const StatFloat
		default_health{ 100.0f },
		default_stamina{ 100.0f },
		default_damage{ 25.0f },
		default_defense{ 10.0f },
		default_fear{ 100.0f, 0.0f },
		default_aggression{ 100.0f, 0.0f };
	inline static const StatUnsigned
		default_aggressionRange{ 3u };

	ActorTemplate(
		const std::optional<DisplayableBase>& displayable,
		const std::optional<int>& factionID = std::nullopt,
		const std::optional<unsigned>& level = std::nullopt,
		const std::optional<std::string>& name = std::nullopt,
		const std::optional<StatFloat>& health = std::nullopt,
		const std::optional<StatFloat>& stamina = std::nullopt,
		const std::optional<StatFloat>& damage = std::nullopt,
		const std::optional<StatFloat>& defense = std::nullopt,
		const std::optional<StatFloat>& fear = std::nullopt,
		const std::optional<StatFloat>& aggression = std::nullopt,
		const std::optional<StatUnsigned>& aggressionRange = std::nullopt
	//	std::optional<std::vector<std::unique_ptr<ItemBase<float>>>>&& items = std::nullopt
	) : displayable{ displayable },
		factionID{ factionID },
		level{ level },
		name{ name },
		health{ health },
		stamina{ stamina },
		damage{ damage },
		defense{ defense },
		fear{ fear },
		aggression{ aggression },
		aggressionRange{ aggressionRange }
	//	items{ std::move(items) }
	{
	}

	DisplayableBase getDisplayableBase() const
	{
		return displayable.value_or(default_displayable);
	}
	StatFloat getHealth() const
	{
		return health.value_or(default_health);
	}
	StatFloat getStamina() const
	{
		return stamina.value_or(default_stamina);
	}
	StatFloat getDamage() const
	{
		return damage.value_or(default_damage);
	}
	StatFloat getDefense() const
	{
		return defense.value_or(default_defense);
	}
	StatFloat getFear() const
	{
		return fear.value_or(default_fear);
	}
	StatFloat getAggression() const
	{
		return health.value_or(default_aggression);
	}
	std::string getName() const
	{
		return name.value_or(default_name);
	}
	unsigned getLevel() const
	{
		return level.value_or(default_level);
	}
	ID getFactionID() const
	{
		return factionID.value_or(default_factionID);
	}
	StatUnsigned getAggressionRange() const
	{
		return aggressionRange.value_or(default_aggressionRange);
	}

	/*bool hasItems() const noexcept
	{
		return items.has_value();
	}*/
};
