#pragma once
#include "../base/BaseAttributes.hpp"

#include <string>
#include <optional>

template<typename T = float>
struct ItemTemplate {
	inline static constexpr const T ZERO{ static_cast<T>(0) };
protected:
	std::optional<std::string> name;
	std::optional<T> modifier;
	std::optional<TargetStat> target;

public:
	inline static const std::string
		default_name{ "Item" };
	inline static const T
		default_modifier{ ZERO };
	inline static const TargetStat
		default_target{ TargetStat::NULL_STAT };

	ItemTemplate(
		const std::optional<std::string>& name = std::nullopt,
		const std::optional<T>& modifier = std::nullopt,
		const std::optional<TargetStat>& target = std::nullopt
	) :
		name{ name },
		modifier{ modifier },
		target{ target }
	{}

	std::string getName() const
	{
		return name.value_or(default_name);
	}
	T getModifier() const
	{
		return modifier.value_or(default_modifier);
	}
	TargetStat getTarget() const
	{
		return target.value_or(default_target);
	}
};
