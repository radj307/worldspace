#pragma once
#include "base/BaseAttributes.hpp"
#include "world/point.h"
#include "world/matrix.hpp"
#include "actors/ActorTemplate.hpp"
#include "actors/ActorBase.hpp"
#include "actors/Actors.h"
#include "actors/Faction.hpp"

#include <typeinfo>

static struct {
	size gridSize{ 30, 30 };
	GeneratorSettings generatorConfig{};

	point minPos{ point{ 0, 0 } + static_cast<int>(generatorConfig.wall_always_on_edge) };
	point maxPos{ gridSize - minPos };

	void setGridSize(const size& newGridSize)
	{
		gridSize = newGridSize;
		minPos = point{ 0, 0 } + static_cast<int>(generatorConfig.wall_always_on_edge);
		maxPos = gridSize - minPos;
	}

	const ID NullID{ UID_Controller.getID() };
	const ID PlayerFactionID{ UID_Controller.getID() };
	const ID EnemyFactionID{ UID_Controller.getID() };
	const ID IndepFactionID{ UID_Controller.getID() };

	Faction factionNull{ NullID };
	Faction factionPlayer{
		PlayerFactionID,
		Faction::RelationMap{
			{ EnemyFactionID, Relation::Hostile },
		{ IndepFactionID, Relation::Neutral },
	}
	};
	Faction factionEnemy{
		EnemyFactionID,
		Faction::RelationMap{
			{ PlayerFactionID, Relation::Hostile },
		{ IndepFactionID, Relation::Neutral },
	}
	};
	Faction factionIndep{
		IndepFactionID,
		Faction::RelationMap{
			{ PlayerFactionID, Relation::Friendly },
		{ EnemyFactionID, Relation::Neutral },
	}
	};

	Faction& getFactionFromID(const ID& id)
	{
		if (factionPlayer == id)
			return factionPlayer;
		else if (factionEnemy == id)
			return factionEnemy;
		else if (factionIndep == id)
			return factionIndep;
		else return factionNull;
	}

	std::vector<ActorTemplate> npc_templates{
		ActorTemplate{
			DisplayableBase{ '*', color::setcolor::cyan },
			factionIndep,
			1u,
			"Chicken",
			StatFloat(30.0f),
			StatFloat(50.0f),
			StatFloat(15.0f),
			StatFloat(0.0f),
			StatFloat{ 10.0f, 0.0f },
			StatFloat{ 0.0f, 0.0f }
		},
		ActorTemplate{
			DisplayableBase{ '*', color::setcolor::cyan },
			factionIndep,
			2u,
			"Ram",
			80.0f,
			150.0f,
			20.0f,
			5.0f,
			StatFloat{ 40.0f, 0.0f },
			StatFloat{ 0.0f, 0.0f }
		}
	};

	std::vector<ActorTemplate> enemy_templates{
		ActorTemplate{
			DisplayableBase{ '?', color::setcolor::red },
			factionEnemy,
			1,
			"Bandit",
			100.0f,
			80.0f,
			10.0f,
			2.5f,
			StatFloat{ 50.0f, 0.0f },
			StatFloat{ 100.0f, 0.0f }
		},
		ActorTemplate{
			DisplayableBase{ '!', color::setcolor::magenta },
			factionEnemy,
			2,
			"Marauder",
			110.0f,
			100.0f,
			20.0f,
			5.0f,
			StatFloat{ 100.0f, 0.0f },
			StatFloat{ 100.0f, 0.0f }
		},
		ActorTemplate{
			DisplayableBase{ '%', color::setcolor{ color::rgb_to_sgr(1.0f, 0.2f, 0.01f) } },
			factionEnemy,
			3,
			"Reaver",
			120.0f,
			190.0f,
			19.6f,
			22.2f,
			StatFloat{ 100.0f, 0.0f },
			StatFloat{ 100.0f, 0.0f }
		}
	};

	ActorTemplate player_template{
		DisplayableBase{ '$', color::setcolor::green },
		factionPlayer,
		1,
		"Player",
		100.0f,
		100.0f,
		25.0f,
		15.0f,
		std::nullopt,
		std::nullopt,
		6u
	};

	size_t generate_npc_count{ 10ull };
	size_t generate_enemy_count{ 20ull };
	double npcDistribRate{ 1.0 }; ///< @brief	This is the value for (lambda) in an exponential distribution function. It is used to select actor templates less often when placed at high (relative) indexes in the ActorTemplate vectors above. The lower the value, the closer to a straight line. The higher the value, the more curved. See this image for a graphic: ( https://en.wikipedia.org/wiki/Exponential_distribution#/media/File:Exponential_cdf.svg ).

	float npcIdleMoveChance{ 33.0f }; ///< @brief	Percentage chance out of 100 that an NPC moves when it doesn't have a target, and there are no nearby hostiles.

	float regenHealth{ 0.0f }; ///< @brief	The amount of health regenerated every time the periodic regen function is called
	float regenStamina{ 5.0f }; ///< @brief	The amount of stamina regenerated every time the periodic regen function is called
} GameConfig;

template<std::derived_from<tile>... AllowTypes>
inline bool tileAllowsMovement(tile* t)
{
	const auto& tType{ typeid(*t) };
	return var::variadic_or(tType == typeid(AllowTypes)...);
}
inline bool tileAllowsMovement(tile* t)
{
	return tileAllowsMovement<floortile, traptile, doortile, containertile>(t);
}

/// @brief	Represents a rectangle with a minimum & maximum point.
using bounds = std::pair<size, size>;

inline static bounds getPlayableBounds()
{
	return{ GameConfig.minPos, GameConfig.maxPos };
}
