/**
 * @file itemstats.h
 * @author radj307
 * @brief Contains the ItemStats struct, which contains common attributes shared by all item types.
 */
#pragma once
#include "faction.h"
#include <sysarch.h>

#include <TermAPI.hpp>

#include <string>
#include <vector>

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
	 * @brief Initializes this item's _faction_lock vector to allow any faction to use this item.
	 */
	void initFactionLock()
	{
		for ( auto i = static_cast<int>( FACTION::PLAYER ); i < static_cast<int>( FACTION::NONE ); i++ ) {
			_faction_lock.push_back(static_cast<FACTION>( i ));
		}
	}

	/**
	 * factionCanUse(FACTION)
	 * @brief Checks if a given faction is allowed to use this item by comparing against the _faction_lock vec.
	 * @param f		 - A faction.
	 * @return true	 - The given faction is allowed to use this item.
	 * @return false - The given faction cannot use this item.
	 */
	[[nodiscard]] bool factionCanUse(const FACTION& f)
	{
		return std::ranges::any_of(_faction_lock.begin(), _faction_lock.end(), [&f](FACTION& fac) -> bool { return fac == f; });
	}

public:

	/**
	 * ItemStats(string, int)
	 * @brief Default constructor that sets the display character to '&' with an undefined color, and allows all factions to use it.
	 * @param name			- This item's name
	 * @param maxUses		- The number of times this item can be used before being removed.
	 */
	ItemStats(std::string name, const int maxUses) : _char('&'), _color(Color::_reset), _name(std::move(name)), _use_count(maxUses)
	{
		// Allow any faction to use
		initFactionLock();
	}
	/**
	 * ItemStats(char, Color, string, int)
	 * @brief Constructor that allows all factions to use this item.
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