#pragma once
#include "../base/BaseAttributes.hpp"
#include "ItemTemplate.hpp"

#include <var.hpp>

#include <optional>

template<var::numeric T>
struct ItemBase {
	std::string name;
	T modifier;
	TargetStat target;

	ItemBase(const std::string& name, const T& modifier, const TargetStat& target) : name{ name }, modifier{ modifier }, target{ target } {}
	ItemBase(const ItemTemplate<T>& t) : name{ t.getName() }, modifier{ t.getModifier() }, target{t.getTarget()} {}
	ItemBase(const ItemBase& o) : name{ o.name }, modifier{ o.modifier }, target{ o.target } {}
	ItemBase(ItemBase&& o) : name{ std::move(o.name) }, modifier{ std::move(o.modifier) }, target{ std::move(o.target) } {}
	virtual ~ItemBase() = default;

	virtual std::unique_ptr<ItemBase<T>> clone() = 0;

	bool appliesToStat(const TargetStat& stat)
	{
		return static_cast<unsigned char>(target & stat) != static_cast<unsigned char>(0);
	}
};

template<var::numeric T = float>
struct WeaponItem : ItemBase<T> {
	WeaponItem(const std::string& name, const T& modifier) : ItemBase<T>(name, modifier) {}
	WeaponItem(const ItemTemplate<T>& t) : ItemBase<T>(t) {}

	std::unique_ptr<WeaponItem<T>> clone() override
	{
		return std::make_unique<WeaponItem<T>>(*this);
	}
};

template<var::numeric T = float>
struct ArmorItem : ItemBase<T> {
	ArmorItem(const std::string& name, const T& modifier) : ItemBase<T>(name, modifier) {}
	ArmorItem(const ItemTemplate<T>& t) : ItemBase<T>(t) {}

	std::unique_ptr<ArmorItem<T>> clone() override
	{
		return std::make_unique<ArmorItem<T>>(*this);
	}
};

using StaticItemType = float;

template<var::numeric T = StaticItemType>
struct StaticItem : ItemBase<T>, Positioned {
	StaticItem(const point& pos, const std::string& name, const T& modifier, const TargetStat& targets) : ItemBase<T>(name, modifier, targets), Positioned(pos) {}

	StaticItem(point&& pos, ItemTemplate<T>&& t) : ItemBase<T>(std::forward<ItemTemplate<T>>(t)), Positioned(std::forward<point>(pos)) {}
	StaticItem(const point& pos, const ItemTemplate<T>& t) : ItemBase<T>(t), Positioned(pos) {}
	virtual ~StaticItem() = default;

	std::unique_ptr<StaticItem<T>> clone() override
	{
		return std::make_unique<StaticItem<T>>(*this);
	}
};

template<var::numeric T = StaticItemType>
struct HealthItem : StaticItem<T> {
	inline static constexpr const T DEFAULT_RESTORE{ 20.0f };
	HealthItem(const point& pos, const T& modifier = DEFAULT_RESTORE, const std::string& name = "Restore Health") : StaticItem<T>(pos, name, modifier) {}

	std::unique_ptr<HealthItem<T>> clone() override
	{
		return std::make_unique<HealthItem<T>>(*this);
	}
};

template<var::numeric T = StaticItemType>
struct StaminaItem : StaticItem<T> {
	inline static constexpr const T DEFAULT_RESTORE{ 20.0f };
	StaminaItem(const point& pos, const T& modifier = DEFAULT_RESTORE, const std::string& name = "Restore Stamina") : StaticItem<T>(pos, name, modifier) {}
	
	std::unique_ptr<StaminaItem<T>> clone() override
	{
		return std::make_unique<StaminaItem<T>>(*this);
	}
};
