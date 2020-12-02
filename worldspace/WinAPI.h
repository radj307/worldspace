#pragma once
#include <iostream>
#include <Windows.h>

/**
 * Windows-Specific API function wrappers.
 */
namespace WinAPI {

    // These are pre-set hex values for the setConsoleColor() function
    enum class color {
        white = 0x0001 | 0x0002 | 0x0004,
        grey = 0,
        yellow = 0x0002 | 0x0004,
        red = 0x0004,
        magenta = 0x0001 | 0x0004,
        blue = 0x0001,
        cyan = 0x0001 | 0x0002,
        green = 0x0002,
    	reset = 07,
    };

    /**
     * setCursorPos(int, int)
     * Sets the cursor's position to a given x/y coordinate, in relation to the origin point top left corner (0,0) ( src: https://stackoverflow.com/a/34843392 )
     * 
     * @param x - Target horizontal-axis position, measured in characters of the screen buffer
     * @param y - Target vertical-axis position, measured in characters of the screen buffer
     */
	inline void setCursorPos(const int x, const int y)
    {
        std::cout.flush();
		const COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    /**
     * hideCursor()
     * Hides the cursor from the terminal.
     * ( src: https://stackoverflow.com/a/34843392 )
     * 
     * @param isVisible - (Default: false) When true, the cursor is visible. When false, the cursor is not shown.
     */
	inline void visibleCursor(const bool isVisible = false)
    {
        CONSOLE_CURSOR_INFO info{};
        info.dwSize = 100;
        info.bVisible = isVisible;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    }

    /**
     * setConsoleColor(unsigned short)
     * Sets the console color to a supported window color
     *
     * @param makeColor - A windows API color
     */
	inline void setConsoleColor(const color makeColor)
    {
        std::cout.flush();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<unsigned short>(makeColor));
    }
	inline void setConsoleColor(const unsigned int color)
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
        // Get the Win32 handle representing standard output.
        // This generally only has to be done once, so we make it static.
        static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

        CONSOLE_SCREEN_BUFFER_INFO csbi;

        // set origin point to top left corner
		const COORD topLeft = { 0, 0 };

        // flush the cout buffer to prevent garbage characters being written
        std::cout.flush();

        // Figure out the current width and height of the console window
        if ( !GetConsoleScreenBufferInfo(hOut, &csbi) ) return;
        DWORD length = csbi.dwSize.X * csbi.dwSize.Y, written;

        // Flood-fill the console with spaces to clear it
        FillConsoleOutputCharacter(hOut, TEXT(' '), length, topLeft, &written);

        // Reset the attributes of every character to the default.
        // This clears all background colour formatting, if any.
        FillConsoleOutputAttribute(hOut, csbi.wAttributes, length, topLeft, &written);

        // Move the cursor back to the top left for the next sequence of writes
        SetConsoleCursorPosition(hOut, topLeft);
    }
}