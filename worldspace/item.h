/**
 * item.h
 * Represents an item that can be possessed by an actor, or dropped on the map
 * by radj307
 */
#pragma once
#include <string>
#include <utility>

// (virtual) item attributes
struct ItemAttributes {
	const std::string _name; // this item's name
	const std::string _desc; // this item's description
	const int _uses;		 // how many times this item can be used before running out. (0 = infinite)

	ItemAttributes(std::string myName, std::string myDescription, const int myMaxUseCount) : _name(std::move(
		myName)), _desc(std::move(myDescription)), _uses(myMaxUseCount) {}
	virtual ~ItemAttributes() = default;
};

// (virtual) item base
struct ItemBase : ItemAttributes {

	ItemBase(const std::string& myName, const std::string& myDescription, const int myMaxUseCount = 0) : ItemAttributes(myName, myDescription, myMaxUseCount) {}
	virtual ~ItemBase() = default;

	virtual int function(...) = 0;
};

struct Weapon : ItemBase {
	const float _min_damage, _max_damage;

	Weapon(const std::string& myName, const std::string& myDescription, const float myMinDamage, const float myMaxDamage) : ItemBase(myName, myDescription, 0), _min_damage(myMinDamage), _max_damage(myMaxDamage) {}
};

struct InstantPotion : ItemBase {
	const float _amount;
};
struct DurationPotion : ItemBase {
	const float _amountPerSecond;
	const float _duration_s;
};

struct InstantPotionHealth : InstantPotion {


};