#pragma once

#include "Frame.h"
#include "PlayerStatBox.h"
#include "Gamespace.h"

/**
 * initConsole()
 * @brief Changes the size & position of the console window, and hides the cursor.
 * @returns bool	- ( true = success ) ( false = Failed to retrieve information about the console window. )
 */
[[nodiscard]] inline bool initConsole(const Coord& window_origin, const Coord& size)
{
	// Get a handle to the console window
	auto* const cw{ GetConsoleWindow() };

	// Lambda function to get the size of characters
	const auto getCharSize(
		[](const HDC& window) -> Coord {
			TEXTMETRIC tx{};
			GetTextMetrics(window, &tx);
			return { tx.tmMaxCharWidth + 4, tx.tmHeight + 4 };
		});
	const auto chSize{ getCharSize(GetDC(cw)) };

	// return result of modifying the window & hiding the cursor
	return MoveWindow(
		cw, window_origin._x,							// Horizontal position of origin on display
		window_origin._y,							// Vertical position of origin on display
		(size._x * 2 + 4) * chSize._x / 2,	// Horizontal size of window
		(size._y + 4) * chSize._y,			// Vertical size of window
		TRUE										// Repaint (Redraws window content)
	) && sys::term::cursorVisible(false);
}

/**
 * @struct FrameBuffer
 * @brief Double-Buffered console rendering using the Frame struct.
 */
class FrameBuffer {
	Gamespace& _game;							///< @brief A reference to the attached gamespace.
	Coord _window_origin,							///< @brief This is the origin point of the console window on the desktop.
		_size;									///< @brief This is the size/bottom-right-corner of the cell.
	bool _initialized{ false },					///< @brief This is used to re-initialize the frame when the game is unpaused.
		_update_stats{ true },					///< @brief This is used to update the player stat box every other frame.
		_console_initialized{ false };			///< @brief This is used to determine whether the console window was initialized or not.
	Coord _origin;								///< @brief This is the origin of the cell in the screen buffer.
	Frame _last;								///< @brief The last frame printed to the console.
	PlayerStatBox _player_stats;				///< @brief Responsible for the player stats display.
	std::vector<std::tuple<Coord, char, unsigned short> > _cache; ///< @brief Contains display information about actors & items.

	void rebuildCache() noexcept;
	void initFrame(bool doCLS = true);
	[[nodiscard]] std::optional<std::pair<char, unsigned short> > checkPos(const Coord& pos) const noexcept;
	[[nodiscard]] std::optional<std::pair<char, unsigned short> > checkPos(long x, long y) const noexcept;
	[[nodiscard]] Frame buildNextFrame(const Coord& origin);

public:
	/**
	 * FrameBuffer(Cell&, Coord&, Coord&)
	 * @brief Instantiate a FrameBuffer display. Throws std::exception if console window did not initialize correctly.
	 * @param gamespace		- Reference to the attached Gamespace instance
	 * @param windowOrigin	- (Default: (1,1)) Position of the window on the monitor
	 * @param showPlayerValues	- (Default: false) When true, displays the raw stat values below the stat bars.
	 */
	explicit FrameBuffer(Gamespace& gamespace, const Coord& windowOrigin = Coord(1, 1), const bool showPlayerValues = false) : _game(gamespace), _window_origin(windowOrigin), _size(gamespace.getCellSize()), _console_initialized(initConsole(_window_origin, _size)), _origin({ sys::term::getScreenBufferCenter()._x - _size._x - 1, sys::term::getScreenBufferCenter()._y - _size._y / 2L - (showPlayerValues ? 4 : 3) - 2 }), _player_stats(&_game.getPlayer(), { _origin._x + _size._x, _origin._y + _size._y + 1 }, showPlayerValues)
	{
		if (!_console_initialized)
			throw std::exception("The console window failed to initialize.");
	}

	void display();
	void deinitialize() noexcept;
};
