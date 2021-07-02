/**
 * @file itemstatic.h
 * Contains the ItemStaticBase struct, and derivative types, which represent static items found around the world that cannot be picked up.
 * @author radj307
 */
#pragma once
#include <string>
#include <utility>

#include "actorbase.h"
#include "itemstats.h"


// Base static item
struct ItemStaticBase : ItemStats {
protected:
	// This static item's position
	Coord _pos;

	/**
	 * virtual func(ActorBase*)
	 * @brief This item's function. This is overridden in derived classes to provide item with various usages.
	 * @param target	- Pointer to a target actor.
	 */
	virtual void func(ActorBase* target) {}

	/**
	 * virtual cond(ActorBase*)
	 * @brief This item's use condition. This is overridden in derived classes to provide item with use logic.
	 * @param target	- Pointer to a target actor.
	 * @return true		- Target actor is able to use this item.
	 * @return false	- Target actor cannot use this item.
	 */
	virtual bool cond(ActorBase* target) { return true; }

public:
	/**
	 * ItemStaticBase(char, Color, string, int, Coord&)
	 * @brief Default constructor that allows all factions to use this item.
	 * @param display		- This item's display character
	 * @param displayColor	- This item's display color
	 * @param myName		- This item's name
	 * @param myUses		- The number of times this item can be used before being removed.
	 * @param myPos			- Ref to a coord position
	 */
	ItemStaticBase(const char display, const unsigned short displayColor, std::string myName, const int myUses, const Coord& myPos) : ItemStats(display, displayColor, std::move(myName), myUses), _pos(myPos) {}

	/**
	 * ItemStaticBase(char, Color, string, int, Coord&, vector<FACTION>)
	 * @brief Constructor that allows locking this item to specific factions.
	 * @param display		- This item's display character
	 * @param displayColor	- This item's display color
	 * @param myName		- This item's name
	 * @param myUses		- The number of times this item can be used before being removed.
	 * @param myPos			- Ref to a coord position
	 * @param lockToFaction	- Vector of factions allowed to use this item
	 */
	ItemStaticBase(const char display, const unsigned short displayColor, std::string myName, const int myUses, const Coord& myPos, std::vector<FACTION> lockToFaction) : ItemStats(display, displayColor, std::move(myName), myUses, std::move(lockToFaction)), _pos(myPos) {}

	/**
	 * ItemStaticBase(ItemStats&, Coord&)
	 * @brief Constructor using a pre-made ItemStats instance.
	 * @param myStats	- Ref to an instance of ItemStats
	 * @param myPos		- Ref to a coord position
	 */
	ItemStaticBase(ItemStats& myStats, const Coord& myPos) : ItemStats(myStats), _pos(myPos) {}

#pragma region DEFAULT
	// Default constructors/destructor/operators
	ItemStaticBase(const ItemStaticBase&) = default;
	ItemStaticBase(ItemStaticBase&&) = default;
	~ItemStaticBase() override = default;
	ItemStaticBase& operator=(const ItemStaticBase&) = default;
	ItemStaticBase& operator=(ItemStaticBase&&) = default;
#pragma endregion DEFAULT

	/**
	 * pos()
	 * @brief Get this item's position
	 * @returns Coord
	 */
	[[nodiscard]] Coord pos() const { return _pos; }
	
	/**
	 * attempt_use(ActorBase*)
	 * @brief Given actor pointer will try to use this item, if actor's faction is allowed.
	 * @param actor	- Pointer to a target actor
	 */
	bool attempt_use(ActorBase* actor)
	{
		// If actor is not a nullptr, itemstatic.has remaining uses, actor can use this item
		if ( actor != nullptr && factionCanUse(actor->faction()) && _use_count > 0 && cond(actor) ) {
			func(actor); // use this item
			--_use_count; // decrement item's remaining use count
			return true;
		}
		return false;
	}

	/**
	 * print()
	 * @brief Prints this item's colorized display character at the current cursor position.
	 */
	void print() const
	{
		sys::colorSet(_color);
		printf("%c", _char);
		sys::colorReset();
	}
};

// Health potion
struct ItemStaticHealth final : ItemStaticBase {
protected:
	int _amount;	// Amount of health to regen

	/**
	 * func(ActorBase*)
	 * @brief Modifies health by _amount
	 * @param target	- Pointer to target actor
	 */
	void func(ActorBase* target) override
	{
		target->modHealth(_amount);
	}

	/**
	 * cond(ActorBase*)
	 * @brief Checks if an actor's health is below max.
	 * @param target	- Pointer to target actor.
	 * @return true		- Target actor is able to use this item.
	 * @return false	- Target actor cannot use this item.
	 */
	bool cond(ActorBase* target) override
	{
		return target->getHealth() < target->getMaxHealth();
	}

public:

	/**
	 * ItemStaticHealth(Coord&, int)
	 * @brief Default constructor that allows all factions to use this item.
	 * @param myPos				- This item's position in the cell.
	 * @param amountRestored	- The amount of health this item restores.
	 */
	ItemStaticHealth(const Coord& myPos, const int amountRestored) : ItemStaticBase('&', Color::_f_red, "Restore Health", 1, myPos), _amount(amountRestored) {}

	/**
	 * ItemStaticHealth(Coord&, int, vector<FACTION>)
	 * @brief Constructor that allows locking this item to specific factions.
	 * @param myPos				- This item's position in the cell.
	 * @param amountRestored	- The amount of health this item restores.
	 * @param lockToFaction		- Vector of factions allowed to use this item.
	 */
	ItemStaticHealth(const Coord& myPos, const int amountRestored, std::vector<FACTION> lockToFaction) : ItemStaticBase('&', Color::_b_red, "Restore Health", 1, myPos, std::move(lockToFaction)), _amount(amountRestored) {}
#pragma region DEFAULT
	// Default constructors/destructor/operators
	ItemStaticHealth(const ItemStaticHealth&) = default;
	ItemStaticHealth(ItemStaticHealth&&) = default;
	~ItemStaticHealth() override = default;
	ItemStaticHealth& operator=(const ItemStaticHealth&) = default;
	ItemStaticHealth& operator=(ItemStaticHealth&&) = default;
#pragma endregion DEFAULT
};

// Stamina potion
struct ItemStaticStamina final : ItemStaticBase {
protected:
	int _amount;	// Amount of stamina to regen

	/**
	 * func(ActorBase*)
	 * @brief Modifies stamina by _amount.
	 * @param target	- Pointer to target actor.
	 */
	void func(ActorBase* target) override
	{
		target->modStamina(_amount);
	}

	/**
	 * cond(ActorBase*)
	 * @brief Checks if an actor's stamina is below max.
	 * @param target	- Pointer to target actor.
	 * @return true		- Target actor is able to use this item.
	 * @return false	- Target actor cannot use this item.
	 */
	bool cond(ActorBase* target) override
	{
		return target->getStamina() < target->getMaxStamina();
	}

public:

	/**
	 * ItemStaticStamina(Coord&, int)
	 * @brief Default constructor that allows all factions to use this item.
	 * @param myPos				- This item's position in the cell.
	 * @param amountRestored	- The amount of stamina this item restores.
	 */
	ItemStaticStamina(const Coord& myPos, const int amountRestored) : ItemStaticBase('&', Color::_b_green, "Restore Health", 1, myPos), _amount(amountRestored) {}

	/**
	 * ItemStaticStamina(Coord&, int, vector<FACTION>)
	 * @brief Constructor that allows locking this item to specific factions.
	 * @param myPos				- This item's position in the cell.
	 * @param amountRestored	- The amount of stamina this item restores.
	 * @param lockToFaction		- Vector of factions allowed to use this item.
	 */
	ItemStaticStamina(const Coord& myPos, const int amountRestored, std::vector<FACTION> lockToFaction) : ItemStaticBase('&', Color::_b_green, "Restore Health", 1, myPos, std::move(lockToFaction)), _amount(amountRestored) {}

#pragma region DEFAULT
	// Default constructors/destructor/operators
	ItemStaticStamina(const ItemStaticStamina&) = default;
	ItemStaticStamina(ItemStaticStamina&&) = default;
	~ItemStaticStamina() override = default;
	ItemStaticStamina& operator=(const ItemStaticStamina&) = default;
	ItemStaticStamina& operator=(ItemStaticStamina&&) = default;
#pragma endregion DEFAULT
};