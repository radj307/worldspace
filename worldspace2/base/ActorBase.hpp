#pragma once
#include "BaseAttributes.hpp"

inline float calc_damage(const float& incomingDamage, const bool& bypassDefense, const float& myDefense, const float& myStamina)
{
	return incomingDamage - ((bypassDefense ? 0 : myDefense) + myStamina / myDefense);
}

struct ActorBase : DisplayableBase, Positionable {
	std::string name;
	StatFloat health;
	StatFloat stamina;
	StatFloat damage;
	StatFloat defense;

	/**
	 * @brief			Actor Base Constructor
	 * @param name		My name.
	 * @param display	My display character.
	 * @param color		My display character's color.
	 * @param maxHP		My maximum and default health.
	 * @param maxSP		My maximum and default stamina.
	 * @param maxDM		My maximum and default damage.
	 * @param maxDF		My maximum and default defense.
	 */
	ActorBase(const std::string& name, const point& position, const char& display, const color::setcolor& color, const float& maxHP, const float& maxSP, const float& maxDM, const float& maxDF) : DisplayableBase(display, color), name{ name }, health{ maxHP }, stamina{ maxSP }, damage{ maxDM }, defense{ maxDF } {}

	virtual void applyDamage(const float& incoming, const bool& bypassDefense = false)
	{
		if (const float& dmg{ calc_damage(incoming, bypassDefense, defense, stamina) }; dmg > 0f)
			health -= dmg;
	}
};

struct Player : ActorBase {

	Player(const point& position, const char& display, const color::setcolor& color, const float& health, const float& stamina, const float& damage, const float& defense) : ActorBase("Player", position, display, color, health, stamina, damage, defense) {}
};

struct NPC : ActorBase {
	StatFloat fear;
	StatFloat aggression;

	NPC(const std::string& name, const point& position, const char& display, const color::setcolor& color, const float& health, const float& stamina, const float& damage, const float& defense, const float& fear, const float& aggression) : ActorBase(name, position, display, color, health, stamina, damage, defense), fear{ fear, 0f }, aggression{ aggression, 0f } {}

	void applyDamage(const float& incoming, const bool& bypassDefense = false) override
	{
		if (const float& dmg{ calc_damage((fear > aggression ? incoming * 1.2f : incoming), bypassDefense, defense, stamina) }; dmg > 0f)
			health -= dmg;
	}
};

struct Enemy : NPC {

	Enemy(const std::string& name, const point& position, const char& display, const color::setcolor& color, const float& health, const float& stamina, const float& damage, const float& defense, const float& fear, const float& aggression) : NPC(name, position, display, color, health, stamina, damage, defense, fear, aggression) {}

};
