/**
 * @file faction.h
 * @author radj307
 * @brief Contains the FACTION enumerator list, and functions related to faction type conversions.
 */
#pragma once
#include <strconv.hpp>
#include <strmanip.hpp>

#include <optional>
#include <vector>

/**
 * @enum FACTION
 * @brief These are all valid factions used in the game. \n
 * These can be used as an actor's faction, or hostile faction targets.
 */
enum class FACTION {
	PLAYER = 0,	// player has faction #0
	ENEMY = 1,	// basic enemy has faction #1
	NEUTRAL = 2,// neutral actor has faction #2
	NONE = 3,	// actors cannot be members of this faction, but it can be used to create passive NPCs
};

/**
 * strToFactions(string&)
 * @brief Converts a string to a vector of factions.
 * This function is mainly used by the INI loader in GameRules.
 * @param str	- Input string containing faction name(s) separated by commas (',').
 * @returns optional<vector<FACTION>>
 */
inline std::optional<std::vector<FACTION>> strToFactions(const std::string& str)
{
	const auto factionVec { // Lambda functor to convert string vector to faction vector
		[&str]() -> std::vector<FACTION> {
			std::vector<FACTION> rv;
			std::stringstream ss { str };
			for ( std::string parse {}; std::getline(ss, parse, ','); parse.clear() ) {
				parse.erase(std::remove_if(parse.begin(), parse.end(), isspace), parse.end());
				parse.erase(std::remove_if(parse.begin(), parse.end(), ispunct), parse.end());
				str::toupper(parse);
				if ( parse == "ENEMY" )			rv.emplace_back(FACTION::ENEMY);
				else if ( parse == "NEUTRAL" )	rv.emplace_back(FACTION::NEUTRAL);
				else if ( parse == "PLAYER" )	rv.emplace_back(FACTION::PLAYER);
			}
			if ( rv.empty() )
				rv.emplace_back(FACTION::NONE);
			return rv;
	}( ) };
	return factionVec.empty() ? std::nullopt : static_cast<std::optional<std::vector<FACTION>>>( factionVec );
}
