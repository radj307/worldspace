#pragma once
#include "base/BaseAttributes.hpp"
#include "world/point.h"
#include "world/matrix.hpp"
#include "actors/ActorTemplate.hpp"

static struct {
	size gridSize{ 30, 30 };
	GeneratorSettings generatorConfig{};

	point minPos{ point{ 0, 0 } + static_cast<int>(generatorConfig.wall_always_on_edge) };
	point maxPos{ gridSize - minPos };

	static void setGridSize(const size& newGridSize)
	{
		GameConfig.gridSize = newGridSize;
		GameConfig.minPos = point{ 0, 0 } + static_cast<int>(GameConfig.generatorConfig.wall_always_on_edge);
		GameConfig.maxPos = GameConfig.gridSize - GameConfig.minPos;
	}

	std::array<Faction, 5> factions{
		Faction{ NULL_FACTION_ID },
		Faction{ 0, { 1 } },
		Faction{ 1, { 0, 2 } },
		Faction{ 2 },
		Faction{ 3 }
	};

	Faction& getFactionFromID(const int& id)
	{
		for (int i{ 1 }; i < factions.size(); ++i)
			if (auto& fac{ factions[i] }; fac == id)
				return fac;
		return factions[0];
	}

	Faction& playerFaction{ factions[1] };
	Faction& enemyFaction{ factions[2] };
	Faction& neutralFaction{ factions[3] };
	Faction& passiveFaction{ factions[4] };

	std::vector<ActorTemplate> npc_templates{
		ActorTemplate{
			DisplayableBase{ '*', color::setcolor::cyan },
			passiveFaction.operator int(),
			1u,
			"Chicken"s,
			StatFloat(30.0f),
			StatFloat(50.0f),
			StatFloat(15.0f),
			StatFloat(0.0f),
			StatFloat{ 10.0f, 0.0f },
			StatFloat{ 0.0f, 0.0f }
	},
		ActorTemplate{
			DisplayableBase{ '*', color::setcolor::cyan },
			neutralFaction,
			2,
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
			enemyFaction,
			1,
			"Bandit"s,
			100.0f,
			80.0f,
			10.0f,
			2.5f,
			StatFloat{ 50.0f, 0.0f },
			StatFloat{ 100.0f, 0.0f }
	},
		ActorTemplate{
			DisplayableBase{ '!', color::setcolor::magenta },
			enemyFaction,
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
			enemyFaction,
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
		playerFaction,
		1,
		"Player",
		100.0f,
		100.0f,
		25.0f,
		25.0f,
		std::nullopt,
		std::nullopt
	};

	size_t generate_npc_count{ 10ull };
	size_t generate_enemy_count{ 20ull };
	double npcDistribRate{ 1.0 }; ///< @brief	This is the value for (lambda) in an exponential distribution function. It is used to select actor templates less often when placed at high (relative) indexes in the ActorTemplate vectors above. The lower the value, the closer to a straight line. The higher the value, the more curved. See this image for a graphic: ( https://en.wikipedia.org/wiki/Exponential_distribution#/media/File:Exponential_cdf.svg ).

	float npcIdleMoveChance{ 33.0f }; ///< @brief	Percentage chance out of 100 that an NPC moves when it doesn't have a target, and there are no nearby hostiles.
} GameConfig;
