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

/**
 * @struct	ActorBase
 * @brief	Base actor object that all other actor types inherit from.
 */
struct ActorBase : DisplayableBase, Positionable {
private:
	std::vector<ActorBase*> isTargetingMe; // list of all actors who have this actor as a target
	ActorBase* myTarget{ nullptr }; // the actor who I am targetting

	/**
	 * @brief			Removes the pointer to the actor with the given ID number from this actor's tracking list.
	 * @param uid		The unique ID number of the actor to remove from the tracking list.
	 */
	void removeTargetingEntry(const unsigned& uid) noexcept
	{
		isTargetingMe.erase(std::remove_if(isTargetingMe.begin(), isTargetingMe.end(), [&uid](ActorBase* ptr) -> bool { return ptr == nullptr || ptr->myID == uid; }), isTargetingMe.end());
	}

public:
	/// @brief	This is my personal tracking ID number.
	unsigned myID{ UID_Controller.getID() };
	/// @brief	This is the ID number of the faction that I belong to.
	ID factionID;
	/// @brief	This is my current level.
	unsigned level;
	/// @brief	This is my name.
	std::string name;
	/// @brief	This determines the amount of health that I have, which determines the amount of damage I can take without dying.
	StatFloat health;
	/// @brief	This determines the amount of stamina I have to attack and defend against attacks against me.
	StatFloat stamina;
	/// @brief	This determines my base damage when attacking other actors.
	StatFloat damage;
	/// @brief	This determines my damage resistance to all non-piercing attacks.
	StatFloat defense;
	/// @brief	This is the distance (in tiles) that I can see in a circular radius around my current position.
	StatUnsigned visRange;
	/// @brief	This is a list of all of the items currently in my possession.
	std::vector<std::unique_ptr<ItemBase<float>>> items;

	/**
	 * @brief			Actor Base Constructor
	 * @param level		My starting level.
	 * @param name		My name.
	 * @param display 	My display character.
	 * @param color		My display character's color.
	 * @param maxHP		My maximum and default health.
	 * @param maxSP		My maximum and default stamina.
	 * @param maxDM		My maximum and default damage.
	 * @param maxDF		My maximum and default defense.
	 */
	ActorBase(const ID& factionID, const unsigned& level, const std::string& name, const point& position, const char& display, const color::setcolor& color, const float& maxHP, const float& maxSP, const float& maxDM, const float& maxDF, const unsigned& visRange, std::vector<std::unique_ptr<ItemBase<float>>> items = {})
		: DisplayableBase(display, color), factionID{ factionID }, level{ level }, name{ name }, health{ maxHP }, stamina{ maxSP }, damage{ maxDM }, defense{ maxDF }, items{ std::move(items) }, visRange{ visRange } {}

	/**
	 * @brief			Template Constructor.
	 * @param startPos	The starting position of this actor.
	 * @param t			The template to use for this actor's stats.
	 */
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

	/// @brief	Destructor that cleans up any pointers to this actor.
	~ActorBase()
	{
		if (!isTargetingMe.empty()) {
			switch (Global.state) { //< this prevents exceptions when shutting down, and also prevents unnecessary cleanup
			case GameState::PAUSED: [[fallthrough]];
			case GameState::RUNNING:
				for (auto it{ isTargetingMe.begin() }; it != isTargetingMe.end(); ++it)
					(*it)->unsetTarget(false);
				break;
			default:break;
			}
		}
	}

	/**
	 * @brief	Retrieve this actor's current target, or `nullptr` if one isn't set.
	 * @returns	ActorBase*
	 */
	[[nodiscard]] ActorBase* getTarget() const { return myTarget; }

	/**
	 * @brief	Retrieve the current position of this actor's target if it has one, or std::nullopt if it doesn't.
	 * @returns	std::optional<point>
	 */
	[[nodiscard]] std::optional<point> getTargetPos() const
	{
		if (myTarget != nullptr)
			return myTarget->getPos();
		return std::nullopt;
	}

	/**
	 * @brief	Check if this actor has a target set.
	 * @returns	bool
	 */
	[[nodiscard]] bool hasTarget() const { return myTarget != nullptr; }

	/**
	 * @brief			Set this actor's target to the given pointer.
	 * @param actor		The actor to set as the target.
	 */
	void setTarget(ActorBase* actor)
	{
		myTarget = actor; // set my target
		myTarget->isTargetingMe.emplace_back(this); // add myself to my target's list of 'haters'
	}

	/**
	 * @brief			Unsets this actors target.
	 * @param recurse	When true, removes this actor from its target's list of actors targetting it.
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
