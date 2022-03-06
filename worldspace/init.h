/**
 * @file init.h
 * @author radj307
 * @brief Contains initialization functions that are required to be run before launching a game. \n
 * Used in game_threads.hpp
 */
#pragma once
#include <INI.hpp>
#include <ostream>
#include <strconv.hpp>
#include <TermAPI.hpp>

#include "controls.h"
#include "GameRules.h"
#include "shared.h"

/**
 * @namespace _internal
 * @brief Contains high-level game functions that should not be accessed outside of the game namespace.
 */
namespace game::_internal {
#pragma region INITIALIZER_FUNC
	/**
	 * initDefaultINI(string&)
	 * @brief Writes a default INI file with the given filename.
	 * @param filename	- The name of the created file
	 * @return true		- Success, default INI config was written to disk.
	 * @return false	- An error occurred while writing the default INI config to disk.
	 */// ReSharper disable once CppInconsistentNaming
	inline bool initDefaultINI(const std::string& filename) noexcept
	{
		// VARIABLES & VALUES
		//const section_map defMap{
		file::INI defMap(file::INI::INIContainer::Map{
			{
				"controls", {
					{ "key_up", str::ctos(_CTRL._KEY_UP) },
					{ "key_down", str::ctos(_CTRL._KEY_DOWN) },
					{ "key_left", str::ctos(_CTRL._KEY_LEFT) },
					{ "key_right", str::ctos(_CTRL._KEY_RIGHT) },
					{ "key_pause", str::ctos(_CTRL._KEY_PAUSE) },
					{ "key_quit", str::ctos(_CTRL._KEY_QUIT) },
				}
			},
			{
				"world", {
					{ "sizeH", "30" },
					{ "sizeV", "30" },
					{ "showAllTiles", "false" },
					{ "showAllWalls", "true" },
					{ "fogOfWar", "true" },
					{ "importFromFile", "" },
					{ "trapDamage", "20" },
					{ "trapDamageIsPercentage", "true" },
				}
			},
			{
				"actors", {
					{ "attackCostStamina", "15" },
					{ "attackBlockChance", "35.0" },
					{ "attackMissChanceFull", "11.0" },
					{ "attackMissChanceDrained", "35.0" },
					{ "npcMoveChance", "6.0" },
					{ "npcMoveChanceAggro", "6.0" },
					{ "npcVisModAggro", "1" },
					{ "multStatsByLevel", "false" },
					{ "regen_time", "2" },
					{ "regen_health", "5" },
					{ "regen_stamina", "10" },
					{ "levelKillThreshold", "2" },
					{ "levelKillMult", "2" },
					{ "levelRestorePercent", "50" },
				}
			},
			{
				"enemy", {
					{ "count", "20" },
					{ "aggroDistance", "3" },
					{ "enable_boss", "true" },
					{ "bossDelayedSpawn", "true" },
				}
			},
			{
				"neutral", {
					{ "count", "12" },
				}
			},
			{
				"player", {
					{ "name", "Player" },
					{ "health", "" },
					{ "stamina", "" },
					{ "damage", "" },
					{ "godmode", "false" },
				}
			},
			{
				"timing", {
					{ "framerate", "75" },
					{ "npc_cycle", "225" },
				}
			},
		} );/*,
		// VARIABLE COMMENTS
		defCommentMap{
			{
				"world", {
					{ "sizeH", "Horizontal World Size" },
					{ "sizeV", "Vertical World Size" },
					{ "showAllTiles", "When true, all tiles are always visible" },
					{ "showAllWalls", "When true, all walls are always visible" },
					{ "fogOfWar", "When true, tile discovery is turned off, and only nearby tiles are visible" },
					{ "importFromFile", "When not blank, attempts to load world from this file" },
					{ "trapDamageIsPercentage", "When true, trapDamage is calculated as a percentage of an actors health" },
				}
			},
			{
				"actors", {
					{ "attackBlockChance", "The chance that an actor will block an attack when they have stamina" },
					{ "attackMissChanceDrained", "The chance that an actor will miss an attack when they are out of stamina" },
					{ "npcMoveChance", "Every NPC cycle, there is a 1 in (x) chance of an NPC moving." },
					{ "npcMoveChanceAggro", "Every NPC cycle, there is a 1 in (x) chance of an NPC moving when they have an active target." },
					{ "regen_time", "Every (x) seconds, all actors regen an amount of their health & stamina" },
					{ "levelRestorePercent", "Every time an actor levels up, this percentage of their stats are regenerated instantly." },
				}
			},
			{
				"timing", {
					{ "framerate", "Target/Max frames per second" },
					{ "npc_cycle", "Time between NPC cycles in milliseconds." },
				}
			},
			{
				"enemy", {
					{ "enable_boss", "When true, a boss is spawned at the end of the game" },
					{ "bossDelayedSpawn", "When true, the boss is spawned after the final challenge is completed. Else, it spawns during." },
				}
			},
		};*/
		try { // Write to file & return result
			return file::INI{ std::forward<decltype(defMap)>(defMap)}.write(filename);
			//	return INI_Defaults{ defMap, defCommentMap }.write(filename);
		} catch (...) { return false; }
	}

	/**
	 * initRuleset(INI&)
	 * @brief Initialize the game ruleset from INI file. If INI file is empty, the default GameRules configuration is used instead.
	 * @param cfg	- INI instance ref, this calls the GameRules constructor if sections [world], [actors], [player], [neutral], and [enemy] exist. Else calls the default gamerules constructor.
	 * @returns GameRules
	 */
	inline GameRules initRuleset(file::INI& cfg) noexcept
	{
		if (cfg.check("world") && cfg.check("actors") && cfg.check("player") && cfg.check("neutral") && cfg.check("enemy")) { // if cfg is not empty
			std::cout << term::debug << "Using GameRules from INI" << std::endl;
			return GameRules(cfg);
		}
		std::cout << term::debug << "Using GameRules from defaults" << std::endl;
		return{}; // else return default GameRules configuration
	}

	/**
	 * initControlSet(INI&)
	 * @brief Initialize the control set from INI file. If INI does not contain a controls section, the default controlset is used instead.
	 * @param cfg	- INI instance ref, only the [controls] section is used.
	 * @returns CONTROLS
	 */
	inline CONTROLS initControlSet(file::INI& cfg) noexcept
	{
		// Check if the INI contains a "controls" section.
		if (cfg.check("controls")) {
			std::cout << term::debug << "Using ControlSet from INI" << std::endl;
			return CONTROLS{ // Initialize controlset from INI
				str::stoc(cfg.getvs("controls", "key_up").value_or(_CTRL._KEY_UP)),
				str::stoc(cfg.getvs("controls", "key_down").value_or(_CTRL._KEY_DOWN)),
				str::stoc(cfg.getvs("controls", "key_left").value_or(_CTRL._KEY_LEFT)),
				str::stoc(cfg.getvs("controls", "key_right").value_or(_CTRL._KEY_RIGHT)),
				str::stoc(cfg.getvs("controls", "key_pause").value_or(_CTRL._KEY_PAUSE)),
				str::stoc(cfg.getvs("controls", "key_quit").value_or(_CTRL._KEY_QUIT))
			};
		}
		std::cout << term::debug << "Using ControlSet from defaults" << std::endl;
		return _CTRL; // else return the default controlset
	}

	/**
	 * initTiming(INI&)
	 * @brief Initialize timing values for the display thread & npc thread. Sets framerate/time & npcCycle time.
	 * @param cfg		- INI instance ref, only the [timing] section is used.
	 * @return true		- Successfully initialized timing values.
	 * @return false	- An exception occurred.
	 */
	inline bool initTiming(file::INI& cfg) noexcept
	{
		try {
			if (setFramerate(str::stoui(cfg.getvs("timing", "framerate").value_or(60u))) && setNPCCycle(str::stoui(cfg.getvs("timing", "npc_cycle").value_or(225u))))
				std::cout << term::debug << "Game timings were set successfully." << std::endl;
			return true;
		} catch (...) {
			setFramerate(60);
			setNPCCycle(225);
			std::cout << term::warn << "Invalid 'INI -> [timing]' settings caused an exception, framerate & npc cycle times were set to default." << std::endl;
			return false;
		}
	}
#pragma endregion INITIALIZER_FUNC
}
