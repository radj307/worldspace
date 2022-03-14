#pragma once
#include "ActorBase.hpp"
#include "Faction.hpp"


struct Player : ActorBase {

	Player(const Faction& faction, const unsigned& level, const point& position, const char& display, const color::setcolor& color, const float& health, const float& stamina, const float& damage, const float& defense) : ActorBase(faction, level, "Player", position, display, color, health, stamina, damage, defense) {}

	Player(const point& startPos, const ActorTemplate& t) : ActorBase(startPos, t) {}

};

struct NPC : ActorBase {
private:
	ActorBase* target;

public:
	StatFloat fear;
	StatFloat aggression;

	NPC(const Faction& faction, const unsigned& level, const std::string& name, const point& position, const char& display, const color::setcolor& color, const float& health, const float& stamina, const float& damage, const float& defense, const float& fear, const float& aggression) : ActorBase(faction, level, name, position, display, color, health, stamina, damage, defense), fear{ fear, 0.0f }, aggression{ aggression, 0.0f } {}

	NPC(const point& startPos, const ActorTemplate& t) : ActorBase(startPos, t), fear{ t.getFear() }, aggression{ t.getAggression() } {}

	bool applyDamageFrom(ActorBase* actor) override
	{
		return applyDamage(actor->damage, isAfraid(), actor);
	}

	bool isAfraid()
	{
		return fear > aggression;
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
};

struct Enemy : NPC {

	Enemy(const Faction& faction, const unsigned& level, const std::string& name, const point& position, const char& display, const color::setcolor& color, const float& health, const float& stamina, const float& damage, const float& defense, const float& fear, const float& aggression) : NPC(faction, level, name, position, display, color, health, stamina, damage, defense, fear, aggression) {}
	Enemy(const point& startPos, const ActorTemplate& t) : NPC(startPos, t) {}

};
