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

struct ItemStats {
protected:
	// Display stats
	char _display;
	WinAPI::color _display_color;
	// General stats
	std::string _name;
	int _use_count;
	std::vector<FACTION> _faction_lock; // List of factions that may use this item

	// This function allows all factions to use this item
	void initFactionLock()
	{
		for ( auto i = static_cast<int>(FACTION::PLAYER); i < static_cast<int>(FACTION::NONE); i++ ) {
			_faction_lock.push_back(static_cast<FACTION>(i));
		}
	}
	// Returns true if the given actor's faction is allowed to use this item
	bool canUse(const FACTION f) { for ( auto it : _faction_lock ) if ( it == f ) return true; return false; }

	virtual ~ItemStats() = default;
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

	[[nodiscard]] int getUses() const { return _use_count; }
};

struct ItemStaticBase : ItemStats {
protected:
	// This static item's position
	Coord _pos;

	virtual void func(ActorBase* target) {}

public:
	ItemStaticBase(const char display, const WinAPI::color displayColor, std::string myName, const int myUses, const Coord& myPos) : ItemStats(display, displayColor, std::move(myName), myUses), _pos(myPos) {}
	ItemStaticBase(const char display, const WinAPI::color displayColor, std::string myName, const int myUses, const Coord& myPos, std::vector<FACTION> lockToFaction) : ItemStats(display, displayColor, std::move(myName), myUses, std::move(lockToFaction)), _pos(myPos) {}
	ItemStaticBase(ItemStats& myStats, const Coord& myPos) : ItemStats(myStats), _pos(myPos) {}
	ItemStaticBase(const ItemStaticBase& o) = default;
	virtual ~ItemStaticBase() = default;

	void attempt_use(ActorBase* a)
	{
		if ( a != nullptr && canUse(a->faction()) ) {
			func(a);
			_use_count--;
		}
	}
	
	[[nodiscard]] Coord pos() const { return _pos; }

	friend std::ostream& operator<<(std::ostream& os, ItemStaticBase& i)
	{
		const auto hwnd{ GetStdHandle(STD_OUTPUT_HANDLE) };
		// set text color to actor's color
		SetConsoleTextAttribute(hwnd, static_cast<unsigned short>(i._display_color));

		// insert actor's character
		os << i._display;

		// reset text color
		SetConsoleTextAttribute(hwnd, 07);
		return os;
	}
};

struct ItemStaticHealth final : ItemStaticBase {
protected:
	int _amount;
	
	void func(ActorBase* target) override
	{
		target->modHealth(_amount);
	}
	
public:
	explicit ItemStaticHealth(const Coord& myPos, const int amountRestored) : ItemStaticBase('&', WinAPI::color::red, "Restore Health", 1, myPos), _amount(amountRestored) {}
	explicit ItemStaticHealth(const Coord& myPos, const int amountRestored, std::vector<FACTION> lockToFaction) : ItemStaticBase('&', WinAPI::color::red, "Restore Health", 1, myPos, std::move(lockToFaction)), _amount(amountRestored) {}
};

struct ItemStaticStamina final : ItemStaticBase {
protected:
	int _amount;
	
	void func(ActorBase* target) override
	{
		target->modStamina(_amount);
	}
	
public:
	explicit ItemStaticStamina(const Coord& myPos, const int amountRestored) : ItemStaticBase('&', WinAPI::color::green, "Restore Health", 1, myPos), _amount(amountRestored) {}
	explicit ItemStaticStamina(const Coord& myPos, const int amountRestored, std::vector<FACTION> lockToFaction) : ItemStaticBase('&', WinAPI::color::green, "Restore Health", 1, myPos, std::move(lockToFaction)), _amount(amountRestored) {}
};