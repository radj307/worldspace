/**
 * item.h
 * Represents an item that can be possessed by an actor, or dropped on the map
 * by radj307
 */
#pragma once
#include <string>

// (virtual) item attributes
struct ItemAttributes {
	const std::string _name; // this item's name
	const std::string _desc; // this item's description
	const int _uses;		 // how many times this item can be used before running out. (0 = infinite)

	ItemAttributes(const std::string myName, const std::string myDescription, const int myMaxUseCount) : _name(myName), _desc(myDescription), _uses(myMaxUseCount) {}
	virtual ~ItemAttributes() {}
};

// (virtual) item base
struct ItemBase : public ItemAttributes {

	ItemBase(const std::string myName, const std::string myDescription, const int myMaxUseCount = 0) : ItemAttributes(myName, myDescription, myMaxUseCount) {}
	virtual ~ItemBase() {}

	virtual int function(...) = 0;
};

struct Weapon : public ItemBase {
	const float _min_damage, _max_damage;

	Weapon(const std::string myName, const std::string myDescription, const float myMinDamage, const float myMaxDamage) : ItemBase(myName, myDescription, 0), _min_damage(myMinDamage), _max_damage(myMaxDamage) {}
};

struct InstantPotion : public ItemBase {
	const float _amount;
};
struct DurationPotion : public ItemBase {
	const float _amountPerSecond;
	const float _duration_s;
};

struct InstantPotionHealth : public InstantPotion {


};