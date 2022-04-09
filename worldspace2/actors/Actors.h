#pragma once
#include "ActorBase.hpp"
#include "Faction.hpp"


struct Player : ActorBase {

	Player(const ID& faction, const unsigned& level, const point& position, const char& display, const color::setcolor& color, const float& health, const float& stamina, const float& damage, const float& defense, const unsigned& visRange) : ActorBase(faction, level, "Player", position, display, color, health, stamina, damage, defense, visRange) {}

	Player(const point& startPos, const ActorTemplate& t) : ActorBase(startPos, t) {}

};

struct NPC : ActorBase {
	StatFloat fear;
	StatFloat aggression;

	NPC(const ID& faction, const unsigned& level, const std::string& name, const point& position, const char& display, const color::setcolor& color, const float& health, const float& stamina, const float& damage, const float& defense, const float& fear, const float& aggression, const unsigned& visRange) : ActorBase(faction, level, name, position, display, color, health, stamina, damage, defense, visRange), fear{ fear, 0.0f }, aggression{ aggression, 0.0f } {}

	NPC(const point& startPos, const ActorTemplate& t) : ActorBase(startPos, t), fear{ t.getFear() }, aggression{ t.getAggression() } {}

	bool applyDamageFrom(ActorBase* actor) override
	{
		return applyDamage(actor->damage, isAfraid(), actor);
	}

	bool isAfraid()
	{
		return fear > aggression;
	}
};

struct Enemy : NPC {

	Enemy(const ID& faction, const unsigned& level, const std::string& name, const point& position, const char& display, const color::setcolor& color, const float& health, const float& stamina, const float& damage, const float& defense, const float& fear, const float& aggression, const unsigned& visRange) : NPC(faction, level, name, position, display, color, health, stamina, damage, defense, fear, aggression, visRange) {}
	Enemy(const point& startPos, const ActorTemplate& t) : NPC(startPos, t) {}

};
