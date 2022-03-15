#pragma once
#include "ActorBase.hpp"
#include "Faction.hpp"
#include "../GameConfig.h"

struct Player : ActorBase {
	StatBaseNoMax<float> experience;
	float threshold{ 20.0f };

	Player(const Faction& faction, const unsigned& level, const point& position, const char& display, const color::setcolor& color, const float& health, const float& stamina, const float& damage, const float& defense, const float& experience = 0.0f) : ActorBase(faction, level, "Player", position, display, color, health, stamina, damage, defense), experience{ experience } {}

	Player(const point& startPos, const ActorTemplate& t) : ActorBase(startPos, t), experience{ t.getExperience() } {}

	void increaseExperienceFrom(ActorBase* actor)
	{
		if (actor->isDead()) {

		}
	}
};

struct NPC : ActorBase {
private:
	ActorBase* target;

public:
	StatFloat fear;
	StatFloat aggression;
	float aggression_rate;
	unsigned aggroDist;

	NPC(const Faction& faction, const unsigned& level, const std::string& name, const point& position, const char& display, const color::setcolor& color, const float& health, const float& stamina, const float& damage, const float& defense, const float& fear, const float& aggression, const float& aggression_rate, const unsigned& aggroDist) : ActorBase(faction, level, name, position, display, color, health, stamina, damage, defense), fear{ fear, 0.0f }, aggression{ aggression, 0.0f }, aggression_rate{ aggression_rate }, aggroDist{ aggroDist } {}

	NPC(const point& startPos, const ActorTemplate& t) : ActorBase(startPos, t), fear{ t.getFear() }, aggression{ t.getAggression() }, aggression_rate{ t.get_aggression_rate() }, aggroDist{ t.getAggroDist() } {}

	bool applyDamageFrom(ActorBase* actor) override
	{
		return applyDamage(actor->damage, isAfraid(), actor);
	}

	float getFearAggressionRatio() const
	{
		return (aggression - fear) / getNPCFearAggressionScale();
	}

	bool isAfraid() const
	{
		return getFearAggressionRatio() > 1.0f;
	}
	bool isAggressive() const
	{
		return getFearAggressionRatio() <= 0.25f;
	}

	bool hasTarget() const
	{
		return target != nullptr;
	}
	ActorBase* getTarget()
	{
		return target;
	}
	void setTarget(ActorBase* newTarget)
	{
		target = newTarget;
	}

	/**
	 * @brief		Check if a given point is within a circle drawn around this point.
	 * @param p		A 2-dimensional point.
	 * @returns		bool
	 */
	bool withinAggroDist(const point& p) const
	{
		return pos.withinCircle(aggroDist, p);
	}

	/**
	 * @brief			Modify the aggression value.
	 * @param amount	The amount to modify the aggression by.
	 */
	void applyAggression(const float& amount)
	{
		aggression += amount * getFearAggressionRatio();
	}
	void applyAggression()
	{
		applyAggression(aggression_rate);
	}

	void increaseAggression()
	{
		applyAggression(aggression + aggression_rate);
	}
	void reduceAggression()
	{
		applyAggression(aggression - aggression_rate);
	}
};

struct Enemy : NPC {

	Enemy(const Faction& faction, const unsigned& level, const std::string& name, const point& position, const char& display, const color::setcolor& color, const float& health, const float& stamina, const float& damage, const float& defense, const float& fear, const float& aggression, const float& aggression_rate, const float& aggroDist) : NPC(faction, level, name, position, display, color, health, stamina, damage, defense, fear, aggression, aggression_rate, aggroDist) {}
	Enemy(const point& startPos, const ActorTemplate& t) : NPC(startPos, t) {}

};
