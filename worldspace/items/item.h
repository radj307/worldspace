/**
 * item.h
 * Represents an item that can be possessed by an actor, or dropped on the map
 * @author radj307
 */
#pragma once
#include <string>
#include <utility>

#include "../actors/actor.h"

 // Base item statistics
struct ItemStats {
protected:
	// Display stats
	char _char;						// A char to represent this item on the display
	color::setcolor _color;		// A windows API color to display this item's character with
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
		for (auto i = static_cast<int>(FACTION::PLAYER); i < static_cast<int>(FACTION::NONE); i++) {
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
	bool canUse(const FACTION f) { for (auto it : _faction_lock) if (it == f) return true; return false; }

public:

	/**
	 * ItemStats(string, int)
	 * @brief Default constructor that sets the display character to '&' with an undefined color, and allows all factions to use it.
	 *
	 * @param name			- This item's name
	 * @param maxUses		- The number of times this item can be used before being removed.
	 */
	ItemStats(std::string name, const int maxUses) : _char('&'), _color(color::reset), _name(std::move(name)), _use_count(maxUses)
	{
		// Allow any faction to use
		initFactionLock();
	}
	/**
	 * ItemStats(char, Color, string, int)
	 * @brief Constructor that allows all factions to use this item.
	 *
	 * @param display		- This item's display character
	 * @param displayColor	- This item's display color
	 * @param name			- This item's name
	 * @param maxUses		- The number of times this item can be used before being removed.
	 */
	ItemStats(const char display, const color::setcolor& displayColor, std::string name, const int maxUses) : _char(display), _color(displayColor), _name(std::move(name)), _use_count(maxUses)
	{
		initFactionLock();
	}
	/**
	 * ItemStats(char, Color, string, int, vector<FACTION>)
	 * @brief Constructor that allows locking this item to specific factions.
	 * @param display		- This item's display character
	 * @param displayColor	- This item's display color
	 * @param name			- This item's name
	 * @param maxUses		- The number of times this item can be used before being removed.
	 * @param canBeUsedBy	- Vector of factions allowed to use this item.
	 */
	ItemStats(const char display, const color::setcolor& displayColor, std::string name, const int maxUses, std::vector<FACTION> canBeUsedBy) : _char(display), _color(displayColor), _name(std::move(name)), _use_count(maxUses), _faction_lock(std::move(canBeUsedBy)) {}

#pragma region DEFAULT
	// Default constructors/destructor/operators
	ItemStats(const ItemStats&) = default;
	ItemStats(ItemStats&&) = default;
	virtual ~ItemStats() = default;
	ItemStats& operator=(const ItemStats&) = default;
	ItemStats& operator=(ItemStats&&) = default;
#pragma endregion DEFAULT

	/**
	 * getUses()
	 * @brief Returns this item's remaining uses.
	 * @returns int
	 */
	[[nodiscard]] int getUses() const { return _use_count; }

	/**
	 * getChar()
	 * @brief Get this item's display character.
	 * @returns char
	 */
	[[nodiscard]] char getChar() const { return _char; }

	/**
	 * getColor()
	 * @brief Get this item's display color.
	 * @returns unsigned short
	 */
	[[nodiscard]] unsigned short getColor() const { return _color; }
};

// Base static item
struct ItemStaticBase : ItemStats {
protected:
	// This static item's position
	Coord _pos;

	/**
	 * virtual func(ActorBase*)
	 * @brief This item's function. This is overridden in derived classes to provide item with various usages.
	 *
	 * @param target	- Pointer to a target actor
	 */
	virtual void func(ActorBase* target) {}

	/**
	 * virtual cond(ActorBase*)
	 * @brief This item's use condition. This is overridden in derived classes to provide item with use logic.
	 *
	 * @param target	- Pointer to a target actor
	 */
	virtual bool cond(ActorBase* target) { return true; }

public:
	/**
	 * ItemStaticBase(char, Color, string, int, Coord&)
	 * @brief Default constructor that allows all factions to use this item.
	 *
	 * @param display		- This item's display character
	 * @param displayColor	- This item's display color
	 * @param myName		- This item's name
	 * @param myUses		- The number of times this item can be used before being removed.
	 * @param myPos			- Ref to a coord position
	 */
	ItemStaticBase(const char display, const color::setcolor& displayColor, std::string myName, const int myUses, const Coord& myPos) : ItemStats(display, displayColor, std::move(myName), myUses), _pos(myPos) {}

	/**
	 * ItemStaticBase(char, Color, string, int, Coord&, vector<FACTION>)
	 * @brief Constructor that allows locking this item to specific factions.
	 *
	 * @param display		- This item's display character
	 * @param displayColor	- This item's display color
	 * @param myName		- This item's name
	 * @param myUses		- The number of times this item can be used before being removed.
	 * @param myPos			- Ref to a coord position
	 * @param lockToFaction	- Vector of factions allowed to use this item
	 */
	ItemStaticBase(const char& display, const color::setcolor& displayColor, std::string myName, const int& myUses, const Coord& myPos, std::vector<FACTION> lockToFaction) : ItemStats(display, displayColor, std::move(myName), myUses, std::move(lockToFaction)), _pos(myPos) {}

	/**
	 * ItemStaticBase(ItemStats&, Coord&)
	 * @brief Constructor using a pre-made ItemStats instance.
	 *
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
	 * attempt_use(ActorBase*)
	 * @brief Given actor pointer will try to use this item, if actor's faction is allowed.
	 * @param actor	- Pointer to a target actor
	 */
	void attempt_use(ActorBase* actor)
	{
		// If actor is not a nullptr, item has remaining uses, actor can use this item
		if (actor != nullptr && canUse(actor->faction()) && _use_count > 0 && cond(actor)) {
			func(actor); // use this item
			_use_count--; // decrement item's remaining use count
		}
	}

	/**
	 * pos()
	 * @brief Get this item's position
	 * @returns Coord
	 */
	[[nodiscard]] Coord pos() const { return _pos; }

	/**
	 * print()
	 * @brief Prints this item's colorized display character at the current cursor position.
	 */
	void print() const
	{
		printf("%s%c%s", _color.as_sequence().c_str(), _char, color::reset.c_str());
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

	/**
	 * cond(ActorBase*)
	 * @brief Checks if an actor's health is below max
	 *
	 * @param target	- Pointer to target actor
	 */
	bool cond(ActorBase* target) override
	{
		return target->getHealth() < target->getMaxHealth();
	}

public:

	/**
	 * ItemStaticHealth(Coord&, int)
	 * @brief Default constructor that allows all factions to use this item
	 *
	 * @param myPos				- This item's position in the cell
	 * @param amountRestored	- The amount of health this item restores
	 */
	ItemStaticHealth(const Coord& myPos, const int amountRestored) : ItemStaticBase('&', color::setcolor(color::red, color::Layer::B), "Restore Health", 1, myPos), _amount(amountRestored) {}

	/**
	 * ItemStaticHealth(Coord&, int, vector<FACTION>)
	 * @brief Constructor that allows locking this item to specific factions
	 *
	 * @param myPos				- This item's position in the cell
	 * @param amountRestored	- The amount of health this item restores
	 * @param lockToFaction		- Vector of factions allowed to use this item
	 */
	ItemStaticHealth(const Coord& myPos, const int amountRestored, std::vector<FACTION> lockToFaction) : ItemStaticBase('&', color::setcolor(color::red, color::Layer::B), "Restore Health", 1, myPos, std::move(lockToFaction)), _amount(amountRestored) {}
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
	 * Modifies stamina by _amount
	 *
	 * @param target	- Pointer to target actor
	 */
	void func(ActorBase* target) override
	{
		target->modStamina(_amount);
	}

	/**
	 * cond(ActorBase*)
	 * @brief Checks if an actor's stamina is below max
	 *
	 * @param target	- Pointer to target actor
	 */
	bool cond(ActorBase* target) override
	{
		return target->getStamina() < target->getMaxStamina();
	}

public:

	/**
	 * ItemStaticStamina(Coord&, int)
	 * @brief Default constructor that allows all factions to use this item
	 *
	 * @param myPos				- This item's position in the cell
	 * @param amountRestored	- The amount of stamina this item restores
	 */
	ItemStaticStamina(const Coord& myPos, const int amountRestored) : ItemStaticBase('&', color::setcolor(color::green, color::Layer::B), "Restore Health", 1, myPos), _amount(amountRestored) {}

	/**
	 * ItemStaticStamina(Coord&, int, vector<FACTION>)
	 * @brief Constructor that allows locking this item to specific factions
	 *
	 * @param myPos				- This item's position in the cell
	 * @param amountRestored	- The amount of stamina this item restores
	 * @param lockToFaction		- Vector of factions allowed to use this item
	 */
	ItemStaticStamina(const Coord& myPos, const int amountRestored, std::vector<FACTION> lockToFaction) : ItemStaticBase('&', color::setcolor(color::green, color::Layer::B), "Restore Health", 1, myPos, std::move(lockToFaction)), _amount(amountRestored) {}

#pragma region DEFAULT
	// Default constructors/destructor/operators
	ItemStaticStamina(const ItemStaticStamina&) = default;
	ItemStaticStamina(ItemStaticStamina&&) = default;
	~ItemStaticStamina() override = default;
	ItemStaticStamina& operator=(const ItemStaticStamina&) = default;
	ItemStaticStamina& operator=(ItemStaticStamina&&) = default;
#pragma endregion DEFAULT
};