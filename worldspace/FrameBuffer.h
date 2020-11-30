#pragma once
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
	Frame() : _frame(), _origin({ 0,0 }) {}

	/** CONSTRUCTOR
	 * Frame(vector<vector<char>>)
	 * Instantiate a pre-made frame
	 */
	Frame(std::vector<std::vector<char>> frameMatrix, Coord frameOrigin = Coord(0, 0)) : _frame(frameMatrix), _origin(frameOrigin) {}

	/**
	 * getSize()
	 * Returns the size of this frame's character matrix
	 *
	 * @returns Coord
	 */
	Coord getSize()
	{
		int Y{ 0 }, X{ 0 };
		for ( auto y = _frame.begin(); y != _frame.end(); y++ ) {
			Y++;
			for ( auto x = (*y).begin(); x != (*y).end(); x++ )
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
	inline void draw(bool spaceColumns = true)
	{
		// use dual-iterators to iterate both the frame, and console position from the origin offset
		for ( int consoleY{ _origin._y }, frameY{ 0 }; consoleY < (_origin._y + (signed)_frame.size()); consoleY++, frameY++ ) {
			for ( int consoleX = _origin._x, frameX{ 0 }; consoleX < (_origin._x + (signed)_frame.at(frameY).size()); consoleX++, frameX++ ) {
				WinAPI::setCursorPos((consoleX * 2), consoleY);		// set the cursor position
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
	static inline Frame buildFromCell(Cell& cell, Coord displayOrigin = Coord(0, 0))
	{
		std::vector<std::vector<char>> matrix;
		for ( int y = 0; y < cell._max._y; y++ ) { // iterate Y-axis
			std::vector<char> row;
			for ( int x = 0; x < cell._max._x; x++ ) { // iterate X-axis
				row.push_back(static_cast<char>(cell.get(x, y)._display));
			}
			matrix.push_back(row);
		}
		return{ matrix, displayOrigin };
	}

	/** STATIC **
	 * buildFromFile(string)
	 * Builds a Frame object from a given file. This function is similar to the importMatrix() function in cell.h, but returns a Frame.
	 *
	 * @param filename	- The name/path to the target file
	 * @returns Frame
	 */
	static inline Frame buildFromFile(std::string filename)
	{
		std::vector<std::vector<char>> matrix{};
		// check if the file exists
		if ( file::exists(filename) ) {
			// copy file content to stringstream
			std::stringstream content{ file::readToStream(filename) };

			content.seekg(0, std::ios::end);
			int content_size{ static_cast<int>(content.tellg()) };
			content.seekg(0, std::ios::beg);

			if ( content_size > 0 ) {
				unsigned int longest_line{ 0 };
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
					for ( unsigned int x = 0; x < line.size(); x++ ) {
						row.push_back(line.at(x));
					}
					matrix.push_back(row);
					line.clear();
				}
				// fill in any lines that are too short with blanks
				for ( auto it = matrix.begin(); it != matrix.end(); it++ ) {
					if ( (*it).size() < longest_line )
						(*it).push_back(' ');
				}
			}
		}
		return{ matrix };
	}

	// Stream insertion operator
	friend inline std::ostream &operator<<(std::ostream &os, const Frame &f)
	{
		for ( auto y = f._frame.begin(); y != f._frame.end(); y++ ) {
			for ( auto x = (*y).begin(); x != (*y).end(); x++ ) {
				os << (*x) << ' ';
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
struct FrameBuffer_Gamespace {
	// The initialized flag, this is set to true once the frame has already been drawn to the console.
	bool _initialized{ false };
	// The last frame printed to the console. (Or the currently displayed frame if this frame-buffer is currently running.)
	Frame _last;
	// The origin-point of this frame, which is the top-left corner of the matrix. Measured in screen buffer characters
	Coord _origin;
	// A reference to the attached gamespace
	Gamespace& _game;

	/** CONSTRUCTOR **
	 * FrameBuffer_Gamespace(Cell&, Coord, vector<ActorBase*>)
	 *
	 * @param cell	 - Ref to a cell
	 * @param origin - Display origin point, measured in chars. This is the top-left corner.
	 * @param vec	 - A vector of pointers to actors
	 */
	FrameBuffer_Gamespace(Gamespace& gamespace, Coord origin) : _game(gamespace), _origin(origin) {}

	/**
	 * getFrame()
	 * Returns a new frame of the entire cell
	 *
	 * @returns Frame
	 */
	inline Frame getFrame(Coord origin)
	{
		std::vector<std::vector<char>> buffer;
		for ( int y = 0; y < _game.getCellSize()._y; y++ ) {
			std::vector<char> row;
			for ( int x = 0; x < _game.getCellSize()._x; x++ ) {
				Coord pos{ Coord(x, y) };
				if ( _game.getTile(pos)._isKnown ) {
					ActorBase *ptr{ _game.getActorAt(pos) };
					if ( ptr != nullptr ) // actor exists at position
						row.push_back(char(ptr->getChar()));
					else
						row.push_back(char(_game.getTile(pos)._display));
				}
				else row.push_back(char(' '));
			}
			buffer.push_back(row);
		}
		return{ buffer, origin };
	}

	/**
	 * initFrame()
	 * Initializes the frame display.
	 * Throws std::exception if cell size is empty.
	 * 
	 * @param origin - This defines the top-left point of the frame, to control its positioning. This should never be manually set outside of the constructor!
	 * @param doCLS	 - (Default: true) When true, the screen buffer is overwritten with blank spaces before initializing the frame.
	 */
	void initFrame(Coord origin, bool doCLS = true)
	{
		if ( !_initialized ) {
			if ( _game.getCellSize()._x > 0 && _game.getCellSize()._y > 0 ) {
				if ( doCLS )
					WinAPI::cls();			// Clear the screen before initializing
				_last = getFrame(origin);	// set the last frame
				_last.draw();		// draw frame
				_initialized = true;	// set init frame boolean
			}
			else throw std::exception("Cannot initialize an empty cell!");
		} // else do nothing
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
			Frame next = getFrame(_origin);
			// iterate vertical axis
			for ( long frameY{ 0 }, consoleY{ _origin._y }; frameY < static_cast<long>(next._frame.size()); frameY++, consoleY++ ) {
				// iterate horizontal axis for each vertical index
				for ( long frameX{ 0 }, consoleX{ _origin._x }; frameX < static_cast<long>(next._frame.at(frameY).size()); frameX++, consoleX++ ) {
					// check if the tile at this pos is known to the player
					if ( _game.getTile(frameX, frameY)._isKnown ) {
						ActorBase* ptr = _game.getActorAt(Coord(frameX, frameY)); // set a pointer to actor at this pos if they exist
						// Check if an actor is located here
						if ( ptr != nullptr ) {
							// set the cursor position to target
							WinAPI::setCursorPos((consoleX * 2), consoleY);
							// output actor with their color
							std::cout << *ptr << ' ';
						}
						// Actor is not located here, show the tile char.
						else if ( next._frame.at(frameY).at(frameX) != _last._frame.at(frameY).at(frameX) ) {
							// set the cursor position to target. (frameX is multiplied by 2 because every other column is blank space)
							WinAPI::setCursorPos((consoleX * 2), consoleY);
							// print next frame's character to position, followed by a blank space.
							std::cout << next._frame.at(frameY).at(frameX) << ' ';
						} // else tile has not changed, do nothing
					}
					// if the tile is not known, show a blank space instead.
					else {
						// set the cursor position to target
						WinAPI::setCursorPos((consoleX * 2), consoleY);
						// output blank
						std::cout << "  ";
					}
				}
			}
			// set the last frame to this frame.
			_last = next;
		}
		else initFrame(_origin);
		playerStatDisplay();
	}

	/**
	 * initConsole()
	 * Changes the size & position of the console window, and hides the cursor.
	 * 
	 * @param windowOriginOnScreen	- Determines the position of the top-left corner of the console in relation to the screen size
	 */
	inline void initConsole(Coord windowOriginOnScreen = Coord(10,10))
	{
		// Get size of each character
		TEXTMETRIC tx{};
		GetTextMetrics(GetDC(GetConsoleWindow()), &tx);
		int ch_width{ tx.tmMaxCharWidth + 5 }, ch_height{ tx.tmHeight + 5 };

		// Get size of the cell
		Coord cellSize{ _game.getCellSize() };

		// move the window, and change its size
		MoveWindow(GetConsoleWindow(), windowOriginOnScreen._x, windowOriginOnScreen._y, ((cellSize._x + (cellSize._x / 4)) * (ch_width)), ((cellSize._y + (cellSize._y / 4)) * (ch_height)), TRUE);

		// hide the cursor
		WinAPI::visibleCursor(false);
	}


	/**
	 * playerStatDisplay()
	 * The player statistics bar
	 */
	inline void playerStatDisplay()
	{
		const std::string HEADER{ "Player Stats" };

		// { (((each box length) + (4 for bar edges & padding)) * (number of stat bars)) }
		int lineLength = 28;// (10 + 4) * 2;

		// calculate the position to display at
		Coord targetDisplayPos{ ((_origin._x + _game.getCellSize()._x) * 2 / 4), ((_origin._y + _game.getCellSize()._y) + (_game.getCellSize()._y / 10)) };
		WinAPI::setCursorPos(targetDisplayPos._x + ((lineLength / 2) - (HEADER.size() / 2)), targetDisplayPos._y);
		std::cout << HEADER;

		// next line
		WinAPI::setCursorPos(targetDisplayPos._x + 1, targetDisplayPos._y + 1);

		// calculate the step value for _game.getPlayer() health
		int segment{ static_cast<int>(_game.getPlayer().getHealth() / 10) };

		// Print the health bar
		std::cout << '[' << termcolor::red;
		for ( int i = 0; i < 10; i++ ) {
			if ( i < segment && segment != 0 )
				std::cout << '@';
			else std::cout << ' ';
		}
		// Print health/stamina bar buffer
		std::cout << termcolor::reset << "]  [" << termcolor::green;

		// calculate the step value for _game.getPlayer() stamina
		segment = { static_cast<int>(_game.getPlayer().getStamina() / 10) };

		// Print the stamina bar
		for ( int i = 0; i < 10; i++ ) {
			if ( i < segment && segment != 0 )
				std::cout << '@';
			else std::cout << ' ';
		}
		// Print stamina bar end buffer
		std::cout << termcolor::reset << ']';

		// Set the cursor position to the next line
		WinAPI::setCursorPos(targetDisplayPos._x + 1, targetDisplayPos._y + 2);

		// get health/stamina values
		int health{ _game.getPlayer().getHealth() }, stamina{ _game.getPlayer().getStamina() };

		// Print health values
		std::cout << "Health: " << termcolor::red << health << termcolor::reset;
		if ( health < 10 )	 std::cout << ' ';
		if ( health < 100 )	 std::cout << ' ';

		// Print stamina values
		std::cout << "\tStamina: " << termcolor::green << stamina << termcolor::reset;
		if ( stamina < 10 )	 std::cout << ' ';
		if ( stamina < 100 ) std::cout << ' ';
	}
};