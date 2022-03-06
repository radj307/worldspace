#pragma once
#include "Frame.h"
#include "PlayerStatBox.h"
#include "Gamespace.h"
#include <TermAPI.hpp>

#include <utility>
#ifdef OS_WIN
#include <Windows.h>
#endif

/**
 * initConsole()
 * @brief Changes the size & position of the console window, and hides the cursor.
 * @returns bool	- ( true = success ) ( false = Failed to retrieve information about the console window. )
 */
[[nodiscard]] bool initConsole(const Coord& window_origin, const Coord& size)
{
#ifdef OS_WIN
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

	//printf(term::CursorVisible(false).c_str());

	// return result of modifying the window & hiding the cursor
	return MoveWindow(
		cw, window_origin._x,							// Horizontal position of origin on display
		window_origin._y,							// Vertical position of origin on display
		(size._x * 2 + 4) * chSize._x / 2,	// Horizontal size of window
		(size._y + 4) * chSize._y,			// Vertical size of window
		TRUE										// Repaint (Redraws window content)
	);
#endif
	return true;
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

	/**
	 * rebuildCache()
	 * @brief Refreshes the cache from current gamespace data.
	 */
	void rebuildCache() noexcept
	{
		//	try {
		const auto size{ _cache.size() };
		_cache.clear();
		_cache.reserve(size);
		for (auto& it : _game.get_all_actors())
			_cache.emplace_back(std::make_tuple(it->pos(), it->getChar(), it->getColor()));
		for (auto& it : _game.get_all_static_items())
			_cache.emplace_back(std::make_tuple(it->pos(), it->getChar(), it->getColor()));
		_cache.shrink_to_fit();
		//	} catch ( ... ) {
		//	}
	}
	/**
	 * initFrame(bool)
	 * @brief Initializes the frame display. Throws make_exception if cell size is empty.
	 * @throws make_exception("Cannot initialize an empty cell!") if the cell is empty.
	 * @param doCLS	 - (Default: true) When true, the screen buffer is overwritten with blank spaces before initializing the frame.
	 */
	void initFrame(bool doCLS = true)
	{
		if (!_initialized) {
			if (_game.getCellSize()._x > 0 && _game.getCellSize()._y > 0) {
				if (doCLS)
					printf(term::clear.c_str());	// Clear the screen before initializing
				_last = buildNextFrame(_origin);	// set the last frame
				_last.draw();						// draw frame
				_initialized = true;				// set init frame boolean
			}
			else
				throw make_exception("Cannot initialize an empty cell!");
		} // else do nothing
	}
	/**
	 * checkPos(Coord&)
	 * @brief Checks a given position for matches in the cache.
	 * @param pos							- Target position.
	 * @return nullopt						- No entities are located at the given position.
	 * @return pair<char, unsigned short>	- The display character & color of the entity located at the given position.
	 */
	[[nodiscard]] std::optional<std::pair<char, unsigned short> > checkPos(const Coord& pos) const noexcept
	{
		for (const auto& it : _cache)
			if (pos == std::get<0>(it))
				return std::make_pair(std::get<1>(it), std::get<2>(it));
		return std::nullopt;
	}
	/**
	 * checkPos(long, long)
	 * @brief Checks a given position for matches in the cache.
	 * @param x								- Target horizontal X-axis position.
	 * @param y								- Target vertical Y-axis position.
	 * @return nullopt						- No entities are located at the given position.
	 * @return pair<char, unsigned short>	- The display character & color of the entity located at the given position.
	 */
	[[nodiscard]] std::optional<std::pair<char, unsigned short> > checkPos(long x, long y) const noexcept
	{
		return checkPos({ x, y });
	}
	/**
	 * buildNextFrame(Coord)
	 * @brief Returns a new frame of the entire cell
	 * @param origin	- The top-left corner of the frame, as shown in the console screen buffer
	 * @returns Frame
	 */
	[[nodiscard]] Frame buildNextFrame(const Coord& origin)
	{
		rebuildCache();
		std::vector<std::vector<char> > buffer;
		//try {
		buffer.reserve(_size._y);
		for (auto y = 0; y < static_cast<signed>(buffer.capacity()); y++) {
			std::vector<char> row;
			row.reserve(_size._x);
			for (auto x = 0; x < static_cast<signed>(row.capacity()); x++) {
				auto pos{ Coord(x, y) };
				if (_game.getTile(pos)->_isKnown) {
					const auto entity{ checkPos(pos) };
					if (entity.has_value())
						row.emplace_back(entity.value().first);
					else
						row.emplace_back(static_cast<char>(_game.getTile(pos)->_display));
				}
				else
					row.emplace_back(' ');
			}
			buffer.emplace_back(row);
		}
		return Frame{ buffer, origin };
		//} catch ( ... ) { return {}; }
	}

public:
	/**
	 * FrameBuffer(Cell&, Coord&, Coord&)
	 * @brief Instantiate a FrameBuffer display. Throws make_exception if console window did not initialize correctly.
	 * @param gamespace		- Reference to the attached Gamespace instance
	 * @param windowOrigin	- (Default: (1,1)) Position of the window on the monitor
	 * @param showPlayerValues	- (Default: false) When true, displays the raw stat values below the stat bars.
	 */
	explicit FrameBuffer(Gamespace& gamespace, const Coord& windowOrigin = Coord(1, 1), const bool showPlayerValues = false) : _game(gamespace), _window_origin(windowOrigin), _size(gamespace.getCellSize()), _console_initialized(initConsole(_window_origin, _size)),
		_origin(0, 0/*{ sys::term::getScreenBufferCenter()._x - _size._x - 1, sys::term::getScreenBufferCenter()._y - _size._y / 2L - (showPlayerValues ? 4 : 3) - 2 }*/), _player_stats(&_game.getPlayer(), { _origin._x + _size._x, _origin._y + _size._y + 1 }, showPlayerValues)
	{
		if (!_console_initialized)
			throw make_exception("The console window failed to initialize.");
		else std::cout << term::CursorVisible(false);
	}

	/**
	 * display()
	 * @brief (Re)Build the frame from current Gamespace data, and cleanup expired entities by calling Gamespace::cleanupDead().
	 */
	void display()
	{
		// flush the stdout buffer
		fflush(stdout);
		// Remove dead actors
		_game.cleanupDead();
		// get a pointer to the game flare
		auto* flare{ _game.getFlare() };
		// Check if the frame is already initialized
		if (_initialized) {
			// flush the output buffer to prevent garbage characters from being displayed.
			// Get the new frame
			auto next{ buildNextFrame(_origin) };
			// iterate vertical axis (frame iterator targets cell coords, console iterator targets screen buffer coords)
			for (long frameY{ 0 }, consoleY{ _origin._y }; frameY < static_cast<long>(next._frame.size()); frameY++, consoleY++) {
				// iterate horizontal axis for each vertical index
				for (long frameX{ 0 }, consoleX{ _origin._x }; frameX < static_cast<long>(next._frame.at(frameY).size()); frameX++, consoleX++) {
					printf(color::reset.c_str());
					// check if the tile at this pos is known to the player
					if (_game.getTile(frameX, frameY)->_isKnown) {
						const auto entity{ checkPos(frameX, frameY) };
						if (entity.has_value()) {
							printf("%s%s%c", term::setCursorPosition(consoleX * 2, consoleY).c_str(), color::setcolor(entity.value().second).as_sequence().c_str(), entity.value().first);
						}
						// Check if the game wants a screen color flare
						else if (flare != nullptr && flare->pattern(frameX, frameY)) {
							// set the cursor position to target. (frameX is multiplied by 2 because every other column is blank space)
							printf(term::setCursorPosition(consoleX * 2, consoleY).c_str());
							if (flare->time() % 2 == 0 && flare->time() != 1) {
								printf("%s%c%s", color::setcolor(flare->color()).as_sequence().c_str(), next._frame.at(frameY).at(frameX), color::reset.c_str());
							}
							else
								printf("%c", next._frame.at(frameY).at(frameX));
						}
						// Selectively update each tile if this tile doesn't match the last frame.
						else if (next._frame.at(frameY).at(frameX) != _last._frame.at(frameY).at(frameX)) {
							printf("%s%c", term::setCursorPosition(consoleX * 2, consoleY).c_str(), next._frame.at(frameY).at(frameX));
						}
					}
					// Selectively update each unknown tile if this tile doesn't match the last frame.
					else if (next._frame.at(frameY).at(frameX) != _last._frame.at(frameY).at(frameX)) {
						printf("%s ", term::setCursorPosition(consoleX * 2, consoleY).c_str());
					}
				}
			}
			// set the last frame to this frame.
			_last = next;
			// Update the player stats box every other frame
			if (_update_stats) {
				// display the player stat bar
				_player_stats.display();
				_update_stats = false;
			}
			else
				_update_stats = true;
			// do flare functions
			if (flare != nullptr) {
				if (flare->time() > 1)
					flare->decrement();
				else
					_game.resetFlare();
			}
		}
		else initFrame(); // if the frame hasn't been initialized, initialize it.
	}

	/**
	 * deinitialize()
	 * @brief Set the initialized flag to false, causing the display to be re-initialized next time display() is called.
	 */
	void deinitialize() noexcept
	{
		_initialized = false;
	}
};
