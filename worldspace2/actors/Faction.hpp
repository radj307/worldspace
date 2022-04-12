#pragma once
#include "../base/BaseAttributes.hpp"
#include "ActorBAse.hpp"
#include "UID_Controller.h"

enum class Relation : unsigned char {
	Neutral,
	Friendly,
	Hostile,
};

struct Faction {
	using RelationMap = std::map<ID, Relation>;
protected:
	const ID myID;

	mutable RelationMap relations;

public:
	Faction(ID const& id) : myID{ id } {}
	Faction(ID const& id, RelationMap&& relationMap) : myID{ id }, relations{ std::move(relationMap) } {}
	Faction(RelationMap&& relationMap) : myID{ UID_Controller.getID() }, relations{ std::move(relationMap) } {}

	void setRelation(const ID& id, const Relation& relation) const
	{
		relations.insert_or_assign(id, relation);
	}
	Relation getRelation(const ID& id) const
	{
		return relations[id];
	}

	void setHostile(const ID& id)
	{
		relations.insert_or_assign(id, Relation::Hostile);
	}

	bool isHostileTo(const ID& id) const
	{
		return relations[id] == Relation::Hostile;
	}
	bool isHostileTo(ActorBase* actor) const
	{
		return relations[actor->myID] == Relation::Hostile || relations[actor->factionID] == Relation::Hostile;
	}

	bool operator==(const ID& id) const
	{
		return myID == id;
	}
	bool operator!=(const ID& id) const
	{
		return myID != id;
	}

	bool operator==(ActorBase* actor) const
	{
		return myID == actor->myID || myID == actor->factionID;
	}
	bool operator!=(ActorBase* actor) const
	{
		return myID != actor->myID && myID != actor->factionID;
	}

	ID getID() const
	{
		return myID;
	}

	operator ID() const { return getID(); }
};

