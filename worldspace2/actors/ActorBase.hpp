#pragma once
#include "../base/BaseAttributes.hpp"
#include "../calc.h"
#include "../items/ItemBase.hpp"
#include "ActorTemplate.hpp"
#include "Faction.hpp"

#include <var.hpp>

#include <typeinfo>

// Actor ID controller object
static struct {
private:
	const unsigned minID{ 0u }, maxID{ 0u - 1u };
	unsigned currentID{ minID };

public:
	unsigned getID() { return ++currentID; }
} UID_Controller;

struct ActorBase : DisplayableBase, Positionable {
private:
	std::vector<ActorBase*> isTargetingMe; // list of all actors who have this actor as a target
	ActorBase* myTarget{ nullptr }; // the actor who I am targetting

	void removeTargetingEntry(const unsigned& uid) noexcept
	{
		try {
			for (size_t i{ 0ull }, len{ isTargetingMe.size() }; i < len; ++i) {
				auto& here{ isTargetingMe.at(i) };
				if (bool isNull{ here == nullptr }; !isNull && here->myID == uid) {
					isTargetingMe.erase(isTargetingMe.begin() + i);
					return;
				}
				else if (isNull) {
					isTargetingMe.erase(isTargetingMe.begin() + i--);
					--len;
				}
			}
		} catch (...) {}
	}

public:
	unsigned myID{ UID_Controller.getID() };
	ID factionID;
	unsigned level;
	std::string name;
	StatFloat health;
	StatFloat stamina;
	StatFloat damage;
	StatFloat defense;

	/// @brief	This determines the visibility range of this actor.
	StatUnsigned visRange;
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
	ActorBase(const ID& factionID, const unsigned& level, const std::string& name, const point& position, const char& display, const color::setcolor& color, const float& maxHP, const float& maxSP, const float& maxDM, const float& maxDF, const unsigned& visRange, std::vector<std::unique_ptr<ItemBase<float>>> items = {})
		: DisplayableBase(display, color), factionID{ factionID }, level{ level }, name{ name }, health{ maxHP }, stamina{ maxSP }, damage{ maxDM }, defense{ maxDF }, items{ std::move(items) }, visRange{ visRange } {}

	ActorBase(const point& startPos, const ActorTemplate& t) :
		DisplayableBase(t.getDisplayableBase()),
		Positionable(startPos),
		factionID{ t.getFactionID() },
		level{ t.getLevel() },
		name{ t.getName() },
		health{ t.getHealth() },
		stamina{ t.getStamina() },
		damage{ t.getDamage() },
		defense{ t.getDefense() },
		visRange{ t.getVisRange() }
	{
	}

	~ActorBase()
	{
		if (!isTargetingMe.empty()) {
			// iterate through all actors who have me as a target, and unset their target.
			for (auto it{ isTargetingMe.begin() }; it != isTargetingMe.end(); ++it)
				(*it)->unsetTarget(false);
		}
	}

	/**
	 * @brief	Retrieve this actor's current target, or `nullptr` if one isn't set.
	 * @returns	ActorBase*
	 */
	ActorBase* getTarget() const { return myTarget; }

	/**
	 * @brief	Retrieve the current position of this actor's target if it has one, or std::nullopt if it doesn't.
	 * @returns	std::optional<point>
	 */
	std::optional<point> getTargetPos() const
	{
		if (myTarget != nullptr)
			return myTarget->getPos();
		return std::nullopt;
	}

	/**
	 * @brief	Check if this actor has a target set.
	 * @returns	bool
	 */
	bool hasTarget() const { return myTarget != nullptr; }

	void setTarget(ActorBase* actor)
	{
		myTarget = actor; // set my target
		myTarget->isTargetingMe.emplace_back(this); // add myself to my target's list of 'haters'
	}

	/**
	 * @brief			Unsets this actors target.
	 */
	void unsetTarget(const bool& recurse = true)
	{
		if (myTarget != nullptr) {
			if (recurse)
				myTarget->removeTargetingEntry(myID);
			myTarget = nullptr; // unset my target
		}
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
			dmg -= 2 * (defense / dmg);
			if (attacker != nullptr)
				attacker->stamina -= defense / CalculationSettings.REDUCE_ATTACKER_STAMINA_LOSS_DIV;
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

	/**
	 * @brief	Check if this actor is dead by checking if its health is less than or equal to 0.
	 * @returns	bool
	 */
	[[nodiscard]] virtual bool isDead() const
	{
		return health <= 0.0f;
	}
	/**
	 * @brief	Instantly kills this actor by settings its health to 0.
	 */
	virtual void kill()
	{
		health = 0.0f;
	}
	/**
	 * @brief		Get the distance from this actor's position to a given point by subtracting this actor's position from the point.
	 * @param p		Another point in 2-dimensional space.
	 * @returns		point resulting from the expression (p - this->pos)
	 */
	[[nodiscard]] point distanceTo(const point& p) const
	{
		return getPos().distanceTo(p);
	}
	/**
	 * @brief		Get the distance from this actor's position to another actor's position by subtracting this actor's position from the other actor's position.
	 * @param actor	Another actor.
	 * @returns		point resulting from the expression (actor->pos - this->pos)
	 */
	[[nodiscard]] point distanceTo(ActorBase* actor) const
	{
		if (actor == nullptr)
			return{ 0ll, 0ll };
		return getPos().distanceTo(actor->getPos());
	}
};

/// @brief	Constraint that only allows types derived from ActorBase.
template<typename T> concept actor = std::derived_from<T, ActorBase>;
