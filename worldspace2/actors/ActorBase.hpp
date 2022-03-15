#pragma once
#include "../base/BaseAttributes.hpp"
#include "../calc.h"
#include "../items/ItemBase.hpp"
#include "ActorTemplate.hpp"

#include <var.hpp>

#include <typeinfo>

struct ActorBase : DisplayableBase, Positionable {
	int factionID;
	unsigned level;
	std::string name;
	StatFloat health;
	StatFloat stamina;
	StatFloat damage;
	StatFloat defense;
	std::vector<std::unique_ptr<ItemBase<float>>> items;

	/**
	 * @brief			Actor Base Constructor
	 * @param level		My starting level.
	 * @param name		My name.
	 * @param display	My display character.
	 * @param color		My display character's color.
	 * @param maxHP		My maximum and default health.
	 * @param maxSP		My maximum and default stamina.
	 * @param maxDM		My maximum and default damage.
	 * @param maxDF		My maximum and default defense.
	 */
	ActorBase(const int& factionID, const unsigned& level, const std::string& name, const point& position, const char& display, const color::setcolor& color, const float& maxHP, const float& maxSP, const float& maxDM, const float& maxDF, std::vector<std::unique_ptr<ItemBase<float>>> items = {}) : DisplayableBase(display, color), factionID{ factionID }, level{ level }, name{ name }, health{ maxHP }, stamina{ maxSP }, damage{ maxDM }, defense{ maxDF }, items{ std::move(items) } {}

	ActorBase(const point& startPos, const ActorTemplate& t) :
		DisplayableBase(t.getDisplayableBase()),
		Positionable(startPos),
		factionID{ t.getFactionID() },
		level{ t.getLevel() },
		name{ t.getName() },
		health{ t.getHealth() },
		stamina{ t.getStamina() },
		damage{ t.getDamage() },
		defense{ t.getDefense() }
	{
	}

	/**
	 * @brief					Apply damage to this actor, and check if they died.
	 * @param incoming			The amount of raw incoming damage.
	 * @param bypassDefense		When true, the actor's defense stat is significantly less important, and will allow more damage.
	 * @returns					bool
	 *							true:	Actor was killed by the applied damage.
	 *							false:	Actor is still alive.
	 */
	virtual bool applyDamage(const float& incoming, const bool& bypassDefense = false, ActorBase* attacker = nullptr)
	{
		float dmg{ incoming };
		if (!bypassDefense) {
			dmg -= defense;
			if (attacker != nullptr) {
				attacker->stamina -= defense / CalculationSettings.REDUCE_ATTACKER_STAMINA_LOSS_DIV;
			}
			if (dmg > 0.0f) {
				dmg -= stamina;
				stamina -= dmg / CalculationSettings.REDUCE_DEFENDER_STAMINA_LOSS_DIV; // reduce stamina by the amount of entirely unblocked damage
			}
		}
		if (dmg < 0.0f)
			dmg = 0.0f;
		health -= dmg / CalculationSettings.REDUCE_DEFENDER_HEALTH_LOSS_DIV;
		return this->isDead();
	}
	/**
	 * @brief			Use the given actor pointer to attack this actor.
	 *\n				This calls the applyDamage() function, with the following rules:
	 *\n				- incoming == actor->damage
	 *					- bypassDefense == false
	 * @param actor		A pointer to another actor.
	 * @returns			bool
	 *					true:	Actor was killed by the applied damage.
	 *					false:	Actor is still alive.
	 */
	virtual bool applyDamageFrom(ActorBase* actor)
	{
		if (actor == nullptr)
			throw make_exception("ActorBase::applyDamageFrom(ActorBase*) failed:  Received null pointer!");
		return applyDamage(actor->damage, false, actor);
	}

	[[nodiscard]] virtual bool isDead() const
	{
		return health <= 0.0f;
	}
	virtual void kill()
	{
		health = 0.0f;
	}

	[[nodiscard]] point distanceTo(const point& p) const
	{
		return pos.distanceTo(p);
	}
	[[nodiscard]] point distanceTo(ActorBase* actor) const
	{
		if (actor == nullptr)
			return{ 0ll, 0ll };
		return pos.distanceTo(actor->pos);
	}

	template<std::derived_from<ItemBase<float>> Item, typename... Args>
	void addItem(Args&&... args)
	{
		items.emplace_back(std::make_unique<Item>(std::forward<Args>(args)...));
	}
	void addItem(ItemBase<float>* item)
	{
		items.emplace_back(item->clone());
	}
	std::vector<std::unique_ptr<ItemBase<float>>>::iterator removeItem(const std::vector<std::unique_ptr<ItemBase<float>>>::const_iterator& iter)
	{
		return items.erase(iter, iter + 1);
	}
	bool removeItem(const std::string& name)
	{
		for (auto it{ items.begin() }; it != items.end(); ++it) {
			if (it->get()->name == name) {
				removeItem(it);
				return true;
			}
		}
		return false;
	}
	template<std::derived_from<ItemBase<float>> Item>
	bool removeItemType()
	{
		const auto& itemType{ typeid(Item) };
		for (auto it{ items.begin() }; it != items.end(); ++it) {
			if (typeid(*it->get()) == itemType) {
				removeItem(it);
				return true;
			}
		}
		return false;
	}
};

/// @brief	Constraint that only allows types derived from ActorBase.
template<typename T> concept actor = std::derived_from<T, ActorBase>;
