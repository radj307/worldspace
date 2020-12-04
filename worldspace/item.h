/**
 * item.h
 * Represents an item that can be possessed by an actor, or dropped on the map
 * by radj307
 */
#pragma once
#include <string>
#include <utility>

#include "actor.h"
#include "WinAPI.h"

// Base item statistics
struct ItemStats {
protected:
	// Display stats
	char _display;						// A char to represent this item on the display
	WinAPI::color _display_color;		// A windows API color to display this item's character with
	// General stats
	std::string _name;					// The name of this item
	int _use_count;						// When this value reaches 0, the item should be deleted.
	std::vector<FACTION> _faction_lock; // List of factions that may use this item

	/**
	 * initFactionLock()
	 * Initializes this item's _faction_lock vector to allow any faction to use this item.
	 */
	void initFactionLock()
	{
		for ( auto i = static_cast<int>(FACTION::PLAYER); i < static_cast<int>(FACTION::NONE); i++ ) {
			_faction_lock.push_back(static_cast<FACTION>(i));
		}
	}

	/**
	 * canUse(FACTION)
	 * Checks if a given faction is allowed to use this item by comparing against the _faction_lock vec.
	 *
	 * @param f		 - A faction
	 * @returns bool - ( true = faction can use item ) ( false = faction cannot use item )
	 */
	bool canUse(const FACTION f) { for ( auto it : _faction_lock ) if ( it == f ) return true; return false; }

public:
	ItemStats(std::string name, const int maxUses) : _display('&'), _display_color(WinAPI::color::reset), _name(std::move(name)), _use_count(maxUses)
	{
		// Allow any faction to use
		initFactionLock();
	}
	ItemStats(const char display, const WinAPI::color displayColor, std::string name, const int maxUses) : _display(display), _display_color(displayColor), _name(std::move(name)), _use_count(maxUses)
	{
		initFactionLock();
	}
	ItemStats(const char display, const WinAPI::color displayColor, std::string name, const int maxUses, std::vector<FACTION> canBeUsedBy) : _display(display), _display_color(displayColor), _name(std::move(name)), _use_count(maxUses), _faction_lock(std::move(canBeUsedBy)) {}
	// Default constructors/destructor/operators
	ItemStats(const ItemStats&) = default;
	ItemStats(ItemStats&&) = default;
	virtual ~ItemStats() = default;
	ItemStats& operator=(const ItemStats&) = default;
	ItemStats& operator=(ItemStats&&) = default;
	
	/**
	 * getUses()
	 * Returns this item's remaining uses.
	 *
	 * @returns int
	 */
	[[nodiscard]] int getUses() const { return _use_count; }
};

// Base static item
struct ItemStaticBase : ItemStats {
protected:
	// This static item's position
	Coord _pos;

	/**
	 * virtual func(ActorBase*)
	 * This item's function. This is overridden in derived classes to provide item with various usages.
	 */
	virtual void func(ActorBase* target) {}

public:
	ItemStaticBase(const char display, const WinAPI::color displayColor, std::string myName, const int myUses, const Coord& myPos) : ItemStats(display, displayColor, std::move(myName), myUses), _pos(myPos) {}
	ItemStaticBase(const char display, const WinAPI::color displayColor, std::string myName, const int myUses, const Coord& myPos, std::vector<FACTION> lockToFaction) : ItemStats(display, displayColor, std::move(myName), myUses, std::move(lockToFaction)), _pos(myPos) {}
	ItemStaticBase(ItemStats& myStats, const Coord& myPos) : ItemStats(myStats), _pos(myPos) {}
	// Default constructors/destructor/operators
	ItemStaticBase(const ItemStaticBase&) = default;
	ItemStaticBase(ItemStaticBase&&) = default;
	virtual ~ItemStaticBase() = default;
	ItemStaticBase& operator=(const ItemStaticBase&) = default;
	ItemStaticBase& operator=(ItemStaticBase&&) = default;
	
	/**
	 * attempt_use(ActorBase*)
	 * Given actor pointer will try to use this item, if actor's faction is allowed.
	 *
	 * @param actor	- Pointer to a target actor
	 */
	void attempt_use(ActorBase* actor)
	{
		// If actor is not a nullptr, item has remaining uses, and actor can use this item
		if ( actor != nullptr && canUse(actor->faction()) && _use_count > 0 ) {
			func(actor); // use this item
			_use_count--; // decrement item's remaining use count
		}
	}

	/**
	 * pos()
	 * Get this item's position
	 *
	 * @returns Coord
	 */
	[[nodiscard]] Coord pos() const { return _pos; }

	// Stream insertion operator
	friend std::ostream& operator<<(std::ostream& os, ItemStaticBase& i)
	{
		auto* const hwnd{ GetStdHandle(STD_OUTPUT_HANDLE) };
		// set text color to actor's color
		SetConsoleTextAttribute(hwnd, static_cast<unsigned short>(i._display_color));

		// insert actor's character
		os << i._display;

		// reset text color
		SetConsoleTextAttribute(hwnd, 07);
		return os;
	}
};

// Health potion
struct ItemStaticHealth final : ItemStaticBase {
protected:
	int _amount;	// Amount of health to regen

	/**
	 * func(ActorBase*)
	 * Modifies health by _amount
	 *
	 * @param target	- Pointer to target actor
	 */
	void func(ActorBase* target) override
	{
		target->modHealth(_amount);
	}
	
public:
	ItemStaticHealth(const Coord& myPos, const int amountRestored) : ItemStaticBase('&', WinAPI::color::red, "Restore Health", 1, myPos), _amount(amountRestored) {}
	ItemStaticHealth(const Coord& myPos, const int amountRestored, std::vector<FACTION> lockToFaction) : ItemStaticBase('&', WinAPI::color::b_red, "Restore Health", 1, myPos, std::move(lockToFaction)), _amount(amountRestored) {}
	// Default constructors/destructor/operators
	ItemStaticHealth(const ItemStaticHealth&) = default;
	ItemStaticHealth(ItemStaticHealth&&) = default;
	~ItemStaticHealth() override = default;
	ItemStaticHealth& operator=(const ItemStaticHealth&) = default;
	ItemStaticHealth& operator=(ItemStaticHealth&&) = default;
};

// Stamina potion
struct ItemStaticStamina final : ItemStaticBase {
protected:
	int _amount;	// Amount of stamina to regen

	/**
	 * func(ActorBase*)
	 * Modifies stamina by _amount
	 *
	 * @param target	- Pointer to target actor
	 */
	void func(ActorBase* target) override
	{
		target->modStamina(_amount);
	}
	
public:
	ItemStaticStamina(const Coord& myPos, const int amountRestored) : ItemStaticBase('&', static_cast<WinAPI::color>(BACKGROUND_GREEN), "Restore Health", 1, myPos), _amount(amountRestored) {}
	ItemStaticStamina(const Coord& myPos, const int amountRestored, std::vector<FACTION> lockToFaction) : ItemStaticBase('&', WinAPI::color::b_green, "Restore Health", 1, myPos, std::move(lockToFaction)), _amount(amountRestored) {}
	// Default constructors/destructor/operators
	ItemStaticStamina(const ItemStaticStamina&) = default;
	ItemStaticStamina(ItemStaticStamina&&) = default;
	~ItemStaticStamina() override = default;
	ItemStaticStamina& operator=(const ItemStaticStamina&) = default;
	ItemStaticStamina& operator=(ItemStaticStamina&&) = default;
};