#pragma once
#include <iostream>
#include <Windows.h>

#include "Coord.h"

/**
 * Windows-Specific API function wrappers.
 */
namespace WinAPI {
	// These are pre-set hex values for the setConsoleColor() function. Enums beginning with "b_" are background colors.
	enum class color {
		b_white = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED,
		b_yellow = BACKGROUND_GREEN | BACKGROUND_RED,
		b_red = BACKGROUND_RED,
		b_magenta = BACKGROUND_BLUE | BACKGROUND_RED,
		b_blue = BACKGROUND_BLUE,
		b_cyan = BACKGROUND_BLUE | BACKGROUND_GREEN,
		b_green = BACKGROUND_GREEN,
		white = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
		yellow = FOREGROUND_GREEN | FOREGROUND_RED,
		red = FOREGROUND_RED,
		magenta = FOREGROUND_BLUE | FOREGROUND_RED,
		blue = FOREGROUND_BLUE,
		cyan = FOREGROUND_BLUE | FOREGROUND_GREEN,
		green = FOREGROUND_GREEN,
		reset = 07,
	};

	/**
	 * VirtualTerminal(DWORD, HANDLE)
	 * @brief Wrapper for Windows Virtual Terminal sequences
	 * @param parameter	- The DWORD parameter to send to the given handle
	 * @param handle	- The target handle
	 * @returns bool	- ( true = SetConsoleMode returned success ) ( false = failed )
	 */
	inline bool VirtualTerminal(const DWORD parameter, const HANDLE handle)
	{
		return SetConsoleMode(handle, parameter);
	}
	
	inline bool enable_virtual_terminal()
	{
		return VirtualTerminal(ENABLE_VIRTUAL_TERMINAL_PROCESSING, GetStdHandle(STD_OUTPUT_HANDLE));
	}

	/**
	 * setCursorPos(int, int)
	 * Sets the cursor's position to a given x/y coordinate, in relation to the origin point top left corner (0,0)
	 *
	 * @param x     - Target horizontal-axis position, measured in characters of the screen buffer
	 * @param y     - Target vertical-axis position, measured in characters of the screen buffer
	 */
	inline void setCursorPos(const int x, const int y)
	{
		std::cout.flush();
		const COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
	}

	 /**
	 * setCursorPos(Coord)
	 * Sets the cursor's position to a given x/y coordinate, in relation to the origin point top left corner (0,0)
	 *
	 * @param pos   - Target position, measured in characters of the screen buffer
	 */
	inline void setCursorPos(const Coord pos)
	{
		std::cout.flush();
		const COORD coord = { static_cast<SHORT>(pos._x), static_cast<SHORT>(pos._y) };
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
	}

	/**
	 * hideCursor()
	 * Changes the cursor's visibility in the terminal
	 *
	 * @param isVisible - (Default: false) When true, the cursor is visible. When false, the cursor is not shown.
	 */
	inline void visibleCursor(const bool isVisible = false)
	{
		// ReSharper disable once CppInitializedValueIsAlwaysRewritten
		CONSOLE_CURSOR_INFO info{};
		info.dwSize = 100;
		info.bVisible = isVisible;
		SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
	}

	/**
	 * setConsoleColor(unsigned short)
	 * Sets the console color to a supported windows terminal color.
	 *
	 * @param color - A windows API color
	 */
	inline void setConsoleColor(const color color)
	{
		std::cout.flush();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<unsigned short>(color));
	}

	/**
	 * setConsoleColor(unsigned short)
	 * Sets the console color to a supported windows terminal color.
	 *
	 * @param color - A windows API color as an unsigned short. This can be found with the "color /?" command in cmd.exe
	 */
	inline void setConsoleColor(const unsigned short color)
	{
		std::cout.flush();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	}

	/**
	 * cls()
	 * Clears the console window.
	 * src https://stackoverflow.com/a/34843392
	 */
	inline void cls()
	{
		try { // catch potential exceptions
			// Get the Win32 handle representing standard output.
			// This generally only has to be done once, so we make it static.
			static auto* const hOut = GetStdHandle(STD_OUTPUT_HANDLE);

			CONSOLE_SCREEN_BUFFER_INFO csbi;

			// set origin point to top left corner
			const COORD topLeft{ 0, 0 };

			// flush the cout buffer to prevent garbage characters being written
			std::cout.flush();

			// Figure out the current width and height of the console window
			if ( !GetConsoleScreenBufferInfo(hOut, &csbi) ) return;
			const DWORD length = csbi.dwSize.X * csbi.dwSize.Y;
			DWORD written;

			// Flood-fill the console with spaces to clear it
			FillConsoleOutputCharacter(hOut, TEXT(' '), length, topLeft, &written);

			// Reset the attributes of every character to the default.
			// This clears all background color formatting, if any.
			FillConsoleOutputAttribute(hOut, csbi.wAttributes, length, topLeft, &written);

			// Move the cursor back to the top left for the next sequence of writes
			SetConsoleCursorPosition(hOut, topLeft);
		} catch( ... ) {}
	}
}