/**
 * @file PlayerStatBox.h
 * @author radj307
 * @brief Contains the PlayerStatBox object, used to display player statistics to the console window. \n
 * Used in FrameBuffer.h
 */
#pragma once
#include <string>

#include "actor.h"
#include "Coord.h"
 /**
  * @struct PlayerStatBox
  * @brief Object used to display player stats.
  */
struct PlayerStatBox final {
private:
	std::string _pName;
	int* _pKills{ nullptr }, * _pLevel{ nullptr }, * _pHealth{ nullptr }, * _pMaxHealth{ nullptr }, * _pStamina{ nullptr }, * _pMaxStamina{ nullptr };
	const bool _SHOW_VALUES;
	const long _MAX_LINE_LENGTH, _LINE_COUNT;
	Coord _origin, _max; // top-left & bottom-right corners
	const std::tuple<char, char, char> _CH_BAR;

public:
	/**
	 * PlayerStatBox(Player*, bool)
	 * @brief Default Constructor
	 *
	 *	\<name\> Stats Level \<level\>
	 *	[@@@@@@@@@@]  [@@@@@@@@@@]
	 *	\<health val\>  \<stamina val\>
	 *		  Kills: \<kills\>
	 *
	 * @param playerPtr		- Pointer to the player
	 * @param center_top	- The top-center point of the box
	 * @param chars			- Characters used for stat bars. { \<open bracket\>, \<fill char\>, \<close bracket\> }
	 * @param showValues	- (Default: false) When true, values are displayed below the stat bars.
	 */
	explicit PlayerStatBox(Player* playerPtr, const Coord center_top, const bool showValues = false, std::tuple<char, char, char> chars = { '[', '@', ']' }) : _pName(playerPtr->name()), _pKills(playerPtr->ptrKills()), _pLevel(playerPtr->ptrLevel()), _pHealth(playerPtr->ptrHealth()), _pMaxHealth(playerPtr->ptrMaxHealth()), _pStamina(playerPtr->ptrStamina()), _pMaxStamina(playerPtr->ptrMaxStamina()), _SHOW_VALUES(showValues), _MAX_LINE_LENGTH(28), _LINE_COUNT(3 + showValues), _origin(center_top._x + 3 - _MAX_LINE_LENGTH / 2, center_top._y), _max(_origin._x + _MAX_LINE_LENGTH, _origin._y + _LINE_COUNT), _CH_BAR(std::move(chars))
	{
	}

	/**
	 * width()
	 * @brief Returns the width of the longest line in the player stat box, in characters
	 * @returns unsigned int
	 */
	[[nodiscard]] unsigned int width() const { return _MAX_LINE_LENGTH; }

	/**
	 * height()
	 * @brief Returns the height of the player stat box, in lines.
	 */
	[[nodiscard]] unsigned int height() const { return _LINE_COUNT; }

	/**
	 * display()
	 * @brief Displays the player stat box at the origin point set in constructor
	 */
	void display() const
	{
		// Simply turns an int* to a string
		const auto str([](const int* integer) -> std::string { return std::to_string(*integer); });
		const auto getStatBar(
			[](const int* max, const int* val, const char fillCh = '@') -> std::string {
				std::string r{ "" };
				const auto seg{ *max / 10 };
				for (auto i{ 1 }; i <= 10; ++i)
					r += *val >= i * seg ? fillCh : ' ';
				return r;
			});
		sys::term::cursorPos(_origin); // Set cursor pos
		std::cout << str::align_center({ _pName + " Stats Level " + str(_pLevel) }, _MAX_LINE_LENGTH);
		sys::term::cursorPos(_origin._x, _origin._y + 1);
		printf("(");
		sys::term::colorSet(Color::_f_red);
		printf("%s", getStatBar(_pMaxHealth, _pHealth).c_str());
		sys::term::colorReset();
		printf(")  (");
		sys::term::colorSet(Color::_f_green);
		printf("%s", getStatBar(_pMaxStamina, _pStamina).c_str());
		sys::term::colorReset();
		printf(")");
		sys::term::cursorPos(_origin._x, _origin._y + 2);
		if (_SHOW_VALUES) {
			std::cout << str::align_center({ "Health: " + str(_pHealth) + "  Stamina: " + str(_pStamina) }, _MAX_LINE_LENGTH);
			sys::term::cursorPos(_origin._x, _origin._y + 3);
		}
		std::cout << str::align_center({ "Kills: " + str(_pKills) }, _MAX_LINE_LENGTH);
	}
};
