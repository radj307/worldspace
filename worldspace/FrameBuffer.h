#pragma once
#include <utility>


#include "Gamespace.h"
#include "group.hpp"

/**
 * struct Frame
 * Represents a single frame shown in the console.
 * Used for output buffering to only change characters that have been modified between frames.
 */
struct Frame {
	// This frame's buffer
	std::vector<std::vector<char>> _frame;
	Coord _origin;

	/** CONSTRUCTOR
	 * Frame()
	 * Instantiate a blank frame.
	 */
	Frame() : _origin({ 0,0 }) {}

	/** CONSTRUCTOR
	 * Frame(vector<vector<char>>)
	 * Instantiate a pre-made frame
	 */
	explicit Frame(std::vector<std::vector<char>> frameMatrix, const Coord& frameOrigin = Coord(0, 0)) : _frame(std::move(frameMatrix)), _origin(frameOrigin) {}

	/**
	 * getSize()
	 * Returns the size of this frame's character matrix
	 *
	 * @returns Coord
	 */
	Coord getSize()
	{
		auto Y{ 0 }, X{ 0 };
		for (auto& y : _frame) {
			Y++;
			for ( auto x = y.begin(); x != y.end(); ++x )
				X++;
		}
		return{ X, Y };
	}

	/**
	 * draw()
	 * Draws this frame to the console at it's origin point.
	 *
	 * @param spaceColumns	- (Default: true) When true, every other column is a space char, to show square frames as squares in the console.
	 */
	void draw(const bool spaceColumns = true)
	{
		// use dual-iterators to iterate both the frame, and console position from the origin offset
		for ( int consoleY{ _origin._y }, frameY{ 0 }; consoleY < _origin._y + static_cast<int>(_frame.size()); consoleY++, frameY++ ) {
			for ( int consoleX = _origin._x, frameX{ 0 }; consoleX < _origin._x + static_cast<int>(_frame.at(frameY).size()); consoleX++, frameX++ ) {
				WinAPI::setCursorPos(consoleX * 2, consoleY);		// set the cursor position
				std::cout << _frame.at(frameY).at(frameX);
				if ( spaceColumns )
					std::cout << ' ';	// draw the frame pos to screen
			}
		}
	}

	/** STATIC **
	 * buildFromCell(Cell&, Coord)
	 * Static function that builds a frame from a given cell. Similar to Gamespace::getFrame(), but does not have knowledge of actors.
	 *
	 * @param cell			- A cell reference to build from
	 * @param displayOrigin	- A point in the console window, measured in characters, that will act as the frame's top left corner.
	 * @returns Frame		- The cell's worldspace as a frame object.
	 */
	static Frame buildFromCell(Cell& cell, const Coord& displayOrigin = Coord(0, 0))
	{
		std::vector<std::vector<char>> matrix;
		for (auto y = 0; y < cell._max._y; y++ ) { // iterate Y-axis
			std::vector<char> row;
			for (auto x = 0; x < cell._max._x; x++ ) { // iterate X-axis
				row.push_back(static_cast<char>(cell.get(x, y)._display));
			}
			matrix.push_back(row);
		}
		return Frame{ matrix, displayOrigin };
	}

	/** STATIC **
	 * buildFromFile(string)
	 * Builds a Frame object from a given file. This function is similar to the importMatrix() function in cell.h, but returns a Frame.
	 *
	 * @param filename	- The name/path to the target file
	 * @returns Frame
	 */
	static Frame buildFromFile(const std::string& filename)
	{
		std::vector<std::vector<char>> matrix{};
		// check if the file exists
		if ( file::exists(filename) ) {
			// copy file content to stringstream
			auto content{ file::readToStream(filename) };

			content.seekg(0, std::ios::end);
			const auto content_size{ static_cast<int>(content.tellg()) };
			content.seekg(0, std::ios::beg);

			if ( content_size > 0 ) {
				size_t longest_line{ 0 };
				std::string line;
				// iterate through content stream
				for ( unsigned int y = 0; std::getline(content, line); y++ ) {
					std::vector<char> row;
					// remove whitespace
					line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
					// find the longest line
					if ( line.size() > longest_line )
						longest_line = line.size();
					// iterate through each line
					for (auto& x : line) {
						row.push_back(x);
					}
					matrix.push_back(row);
					line.clear();
				}
				// fill in any lines that are too short with blanks
				for (auto& it : matrix) {
					if (it.size() < longest_line ) it.push_back(' ');
				}
			}
		}
		return Frame{ matrix };
	}

	// Stream insertion operator
	friend std::ostream &operator<<(std::ostream &os, const Frame &f)
	{
		for (const auto& y : f._frame) {
			for ( auto x = y.begin(); x != y.end(); ++x ) {
				os << *x << ' ';
			}
			os << std::endl;
		}
		return os;
	}
};

/**
 * struct FrameBuffer_Gamespace
 * Double-Buffered console rendering using the Frame struct.
 */
class FrameBuffer_Gamespace {
	// The initialized flag, this is set to true once the frame has already been drawn to the console.
	bool _initialized{ false };
	// The last frame printed to the console. (Or the currently displayed frame if this frame-buffer is currently running.)
	Frame _last;
	// The origin-point of this frame, which is the top-left corner of the matrix. Measured in screen buffer characters
	Coord _origin;
	// A reference to the attached gamespace
	Gamespace& _game;

	/**
	 * initConsole()
	 * Changes the size & position of the console window, and hides the cursor.
	 */
	void initConsole() const
	{
		// Get size of each character
		TEXTMETRIC tx{};
		GetTextMetrics(GetDC(GetConsoleWindow()), &tx);
		const int ch_width{ tx.tmMaxCharWidth + 5 }, ch_height{ tx.tmHeight + 5 };

		// Get size of the cell
		const auto cellSize{ _game.getCellSize() };

		// move the window, and change its size
		MoveWindow(GetConsoleWindow(), _origin._x, _origin._y, (cellSize._x + cellSize._x / 4) * ch_width, (cellSize._y + cellSize._y / 4) * ch_height, TRUE);

		// hide the cursor
		WinAPI::visibleCursor(false);
	}

	/**
	 * initFrame(bool)
	 * Initializes the frame display.
	 * Throws std::exception if cell size is empty.
	 *
	 * @param doCLS	 - (Default: true) When true, the screen buffer is overwritten with blank spaces before initializing the frame.
	 */
	void initFrame(const bool doCLS = true)
	{
		if ( !_initialized ) {
			if ( _game.getCellSize()._x > 0 && _game.getCellSize()._y > 0 ) {
				if ( doCLS )
					WinAPI::cls();			// Clear the screen before initializing
				_last = getFrame(_origin);	// set the last frame
				_last.draw();		// draw frame
				_initialized = true;	// set init frame boolean
			}
			else throw std::exception("Cannot initialize an empty cell!");
		} // else do nothing
	}

	/**
	 * getFrame(Coord)
	 * Returns a new frame of the entire cell
	 *
	 * @param origin	- The top-left corner of the frame, as shown in the console screen buffer
	 * @returns Frame
	 */
	[[nodiscard]] Frame getFrame(const Coord& origin) const
	{
		std::vector<std::vector<char>> buffer;
		for ( auto y = 0; y < _game.getCellSize()._y; y++ ) {
			std::vector<char> row;
			for ( auto x = 0; x < _game.getCellSize()._x; x++ ) {
				auto pos{ Coord(x, y) };
				if ( _game.getTile(pos)._isKnown ) {
					auto* const ptr{ _game.getActorAt(pos) };
					if ( ptr != nullptr && !ptr->isDead() ) // actor exists at position
						row.push_back(static_cast<char>(ptr->getChar()));
					else
						row.push_back(static_cast<char>(_game.getTile(pos)._display));
				}
				else row.push_back(static_cast<char>(' '));
			}
			buffer.push_back(row);
		}
		return Frame{ buffer, origin };
	}

	// The level-up flare pattern
	[[nodiscard]] static constexpr bool flare_pattern(const int x, const int y) { return (x - y % 2) % 2 == 0; }

	/**
	 * playerStatDisplay()
	 * The player statistics bar
	 */
	void playerStatDisplay() const
	{
		const auto HEADER{ _game.getPlayer().name() + " Stats Level " + std::to_string(_game.getPlayer().getLevel()) };

		// { (((each box length) + (4 for bar edges & padding)) * (number of stat bars)) }
		const auto lineLength = 28;// (10 + 4) * 2;

		// calculate the position to display at
		const Coord targetDisplayPos{ ((_origin._x + _game.getCellSize()._x) * 2 / 4), (_origin._y + _game.getCellSize()._y + _game.getCellSize()._y / 10) };
		WinAPI::setCursorPos(lineLength / 2 - static_cast<signed>(HEADER.size()) / 2 + static_cast<int>(targetDisplayPos._x), static_cast<int>(targetDisplayPos._y));
		std::cout << termcolor::reset << HEADER;

		// next line
		WinAPI::setCursorPos(targetDisplayPos._x + 1, targetDisplayPos._y + 1);

		// calculate the step value for _game.getPlayer() health
		auto segment{ _game.getPlayer().getMaxHealth() / 10 };

		// Print the health bar
		std::cout << '[' << termcolor::red;
		for ( auto i = 1; i <= 10; i++ ) {
			if ( _game.getPlayer().getHealth() >= (i * segment) )
				std::cout << '@';
			else std::cout << ' ';
		}
		// Print health/stamina bar buffer
		std::cout << termcolor::reset << "]  [" << termcolor::green;

		// calculate the step value for _game.getPlayer() stamina
		segment = { _game.getPlayer().getMaxStamina() / 10 };

		// Print the stamina bar
		for ( auto i = 1; i <= 10; i++ ) {
			if ( _game.getPlayer().getStamina() >= (i * segment) )
				std::cout << '@';
			else std::cout << ' ';
		}
		// Print stamina bar end buffer
		std::cout << termcolor::reset << ']';

		// Set the cursor position to the next line
		WinAPI::setCursorPos(targetDisplayPos._x + 1, targetDisplayPos._y + 2);

		// get health/stamina values
		const auto health{ _game.getPlayer().getHealth() }, stamina{ _game.getPlayer().getStamina() };

		// Print health values
		std::cout << "Health: " << termcolor::red << health << termcolor::reset;
		if ( health < 10 )	 std::cout << ' ';
		if ( health < 100 )	 std::cout << ' ';

		// Print stamina values
		std::cout << "\tStamina: " << termcolor::green << stamina << termcolor::reset;
		if ( stamina < 10 )	 std::cout << ' ';
		if ( stamina < 100 ) std::cout << ' ';

		// Set the cursor position to the next line
		WinAPI::setCursorPos(targetDisplayPos._x + 1, targetDisplayPos._y + 3);

		std::cout << "Kills: " << termcolor::red << _game.getPlayer().getKills() << termcolor::reset;

		// Set the cursor position to the next line
		//WinAPI::setCursorPos(targetDisplayPos._x + 1, targetDisplayPos._y + 3);
		//std::cout << "Next Level: " << ((_game.getRuleset()._level_up_kills * (_game.getRuleset()._level_up_mult * _game.getPlayer().getLevel()))_game.getPlayer().getKills());

		std::cout.flush();
	}

public:

	/** CONSTRUCTOR **
	 * FrameBuffer_Gamespace(Cell&, Coord, vector<ActorBase*>)
	 *
	 * @param gamespace	- Reference to the attached Gamespace instance
	 * @param origin	- Display origin point, measured in chars. This is the top-left corner.
	 */
	FrameBuffer_Gamespace(Gamespace& gamespace, const Coord& origin) : _origin(origin), _game(gamespace)
	{
		initConsole();
	}

	/**
	 * display()
	 * Update the currently drawn frame, will automatically initialize the frame if necessary.
	 * This function will also perform the cleanupDead() function each time it is called.
	 *
	 * The console size must be changed before calling this function for the display to work correctly!
	 * ( initConsole() will do this automatically )
	 */
	void display()
	{
		// Remove dead actors
		_game.cleanupDead();
		// Check if the frame is already initialized
		if ( _initialized ) {
			// flush the output buffer to prevent garbage characters from being displayed.
			std::cout.flush();
			// Get the new frame
			auto next = getFrame(_origin);
			// iterate vertical axis
			for ( long frameY{ 0 }, consoleY{ _origin._y }; frameY < static_cast<long>(next._frame.size()); frameY++, consoleY++ ) {
				// iterate horizontal axis for each vertical index
				for ( long frameX{ 0 }, consoleX{ _origin._x }; frameX < static_cast<long>(next._frame.at(frameY).size()); frameX++, consoleX++ ) {
					// check if the tile at this pos is known to the player
					if ( _game.getTile(frameX, frameY)._isKnown ) {
						auto* const actor = _game.getActorAt(frameX, frameY); // set a pointer to actor at this pos if they exist
						auto* const item = _game.getItemAt(frameX, frameY);
						// Check if an actor is located here
						if ( actor != nullptr ) {
							// set the cursor position to target
							WinAPI::setCursorPos(consoleX * 2, consoleY);
							// output actor with their color
							std::cout << *actor << ' ';
						}
						else if ( item != nullptr ) {
							// set the cursor position to target
							WinAPI::setCursorPos(consoleX * 2, consoleY);
							// output item with their color
							std::cout << *item << ' ';
						}
						// Check if the game wants a screen color flare
						else if ( _game._flare > 0 && flare_pattern(frameX, frameY) ) {
							// set the cursor position to target. (frameX is multiplied by 2 because every other column is blank space)
							WinAPI::setCursorPos(consoleX * 2, consoleY);
							if ( _game._flare % 2 == 0 && _game._flare != 1 ) {
								WinAPI::setConsoleColor(_game._flare_color);
								std::cout << next._frame.at(frameY).at(frameX);
								setConsoleColor(WinAPI::color::reset);
								std::cout << ' ';
							}
							else {
								setConsoleColor(WinAPI::color::reset);
								std::cout << next._frame.at(frameY).at(frameX) << ' ';
							}
						}
						// Actor is not located here, show the tile char.
						else if ( next._frame.at(frameY).at(frameX) != _last._frame.at(frameY).at(frameX) ) {
							// set the cursor position to target. (frameX is multiplied by 2 because every other column is blank space)
							WinAPI::setCursorPos(consoleX * 2, consoleY);
					//		WinAPI::setConsoleColor(_game.getTile(frameX, frameY)._myColor);
							// print next frame's character to position, followed by a blank space.
							std::cout << next._frame.at(frameY).at(frameX) << ' ';
					//		WinAPI::setConsoleColor(WinAPI::color::reset);
						} // else tile has not changed, do nothing
					}
					// if the tile is not known, show a blank space instead.
					else {
						// set the cursor position to target
						WinAPI::setCursorPos(consoleX * 2, consoleY);
						// output blank
						std::cout << "  ";
					}
				}
			}
			// set the last frame to this frame.
			_last = next;
		}
		else initFrame();
		if ( _game._flare > 0 )
			_game._flare--;
		playerStatDisplay();
	}

	/**
	 * deinitialize()
	 * De-Initialize this framebuffer. Next time display() is called, it will reinitialize the frame first.
	 */
	void deinitialize()
	{
		_initialized = false;
	}
};