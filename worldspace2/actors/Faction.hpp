#pragma once
#include <typeinfo>
#include <vector>
#include <algorithm>

using ID = short;

inline constexpr const ID NULL_FACTION_ID{ -1 };

struct Faction {
protected:
	const ID myID;
	std::vector<ID> hostile;
	const bool allowHostileToSelf;

	std::vector<ID>::iterator find(const ID& id)
	{
		return std::find(hostile.begin(), hostile.end(), id);
	}

	bool contains(const ID& id) const
	{
		return std::any_of(hostile.begin(), hostile.end(), [&id](auto&& oid) {return id == oid; });
	}

public:
	Faction(ID&& id, std::vector<ID>&& hostile_ids = {}, const bool& allowHostileToSelf = false) : myID{ std::forward<ID>(id) }, hostile{ std::forward<std::vector<ID>>(hostile_ids) }, allowHostileToSelf{ allowHostileToSelf } {}
	Faction(const ID& id, const std::vector<ID>& hostile_ids = {}, const bool& allowHostileToSelf = false) : myID{ id }, hostile{ hostile_ids }, allowHostileToSelf{ allowHostileToSelf } {}

	void addHostile(const ID& id)
	{
		if ((id != myID || allowHostileToSelf) && !std::any_of(hostile.begin(), hostile.end(), [&id](auto&& oid) {return id == oid; }))
			hostile.emplace_back(id);
	}
	void removeHostile(const ID& id)
	{
		if (const auto& it{ find(id) }; it != hostile.end())
			hostile.erase(it, it);
	}

	bool isHostileTo(const ID& id) const
	{
		return contains(id);
	}

	bool operator==(const ID& id) const
	{
		return myID == id;
	}
	bool operator!=(const ID& id) const
	{
		return myID != id;
	}

	bool isHostileToSelf() const
	{
		return isHostileTo(myID);
	}
	bool mayBeHostileToSelf() const
	{
		return allowHostileToSelf;
	}

	ID getID() const
	{
		return myID;
	}

	operator ID() const { return getID(); }
};
