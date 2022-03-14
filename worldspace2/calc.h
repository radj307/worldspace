/**
 * @file	calc.h
 * @author	radj307
 * @brief	This file contains functions used for calculating various game values & applying scaling, such as damage.
 *\n		It also contains the configurable settings used to modify these calculations, stored in the static global CalculationSettings object.
 */
#pragma once

#define DEBUG_SPECIAL_HANDLING

static struct {
	float REDUCE_DEFENDER_HEALTH_LOSS_DIV{ 1.0f };
	float REDUCE_DEFENDER_STAMINA_LOSS_DIV{ 1.0f };
	float REDUCE_ATTACKER_STAMINA_LOSS_DIV{ 1.0f };
	float LEVEL_MULT{ -0.008 };
	unsigned LEVEL_OFFSET{ 20 };
} CalculationSettings;



/**
 * @brief
 * @param indamage		The attacker's damage value.
 * @param armorpierce	When true, the defender's defense stat is less effective
 * @param defense
 * @param stamina
 * @returns
 */
inline float calc_damage(const float& indamage, const bool& armorpierce, const float& defense, const float& stamina)
{
#ifdef DEBUG_SPECIAL_HANDLING
	const auto& out{ indamage - defense * static_cast<float>(1 - !!armorpierce) };
	return out > 0.0f ? out : 0.0f;
#else
	const float& out{ (indamage - (defense * static_cast<float>(1 - !!armorpierce))) - stamina / CalculationSettings.DAMAGE_STAMINA_DIVISOR };
	return (out > 0.0f ? out : 0.0f);
#endif
}

inline float calc_levelmult(const unsigned& level)
{
	return CalculationSettings.LEVEL_MULT * std::pow(static_cast<float>(level), 2.0f) + CalculationSettings.LEVEL_OFFSET;
}
