#pragma once
#include <typeinfo>
#include <vector>
#include <algorithm>

inline constexpr const int NULL_FACTION_ID{ -1 };

struct Faction {
protected:
	const int myID;

	std::vector<int> hostile;

	auto find(const int& id)
	{
		return std::find(hostile.begin(), hostile.end(), id);
	}

	auto contains(const int& id) const
	{
		return std::any_of(hostile.begin(), hostile.end(), [&id](auto&& oid) {return id == oid; });
	}

public:
	Faction(int&& id, std::vector<int>&& hostile_ids = {}) : myID{std::forward<int>(id)}, hostile{std::forward<std::vector<int>>(hostile_ids)} {}
	Faction(const int& id, const std::vector<int>& hostile_ids = {}) : myID{id}, hostile{hostile_ids} {}

	void setHostile(const int& id)
	{
		if (id != myID && !std::any_of(hostile.begin(), hostile.end(), [&id](auto&& oid) {return id == oid; }))
			hostile.emplace_back(id);
	}
	void unsetHostile(const int& id)
	{
		if (const auto& it{ find(id) }; it != hostile.end())
			hostile.erase(it, it);
	}

	bool isHostileTo(const int& id) const
	{
		return contains(id);
	}
	bool isNeutralTo(const int& id) const
	{
		return !contains(id);
	}

	bool operator==(const int& id) const
	{
		return myID == id;
	}
	bool operator!=(const int& id) const
	{
		return myID != id;
	}

	operator int() const { return myID; }
};
