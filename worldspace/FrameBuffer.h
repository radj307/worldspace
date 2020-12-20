#pragma once
#include <utility>

#include "Gamespace.h"
#include "sysapi.h"

/**
 * struct Frame
 * Represents a single frame shown in the console.
 * Used for output buffering to only change characters that have been modified between frames.
 */
struct Frame {
	// This frame's buffer
	std::vector<std::vector<char>> _frame;
	Coord _origin;
	bool _space_columns;

	/** CONSTRUCTOR
	 * Frame()
	 * @brief Instantiate a blank frame.
	 */
	Frame() : _origin({ 0,0 }), _space_columns(false) {}

	/** CONSTRUCTOR
	 * Frame(vector<vector<char>>)
	 * @brief Instantiate a pre-made frame
	 */
	explicit Frame(std::vector<std::vector<char>> frameMatrix, const Coord& frameOrigin = Coord(0, 0), const bool formatWithSpaces = true) : _frame(std::move(frameMatrix)), _origin(frameOrigin), _space_columns(formatWithSpaces) {}

	/**
	 * isValidSize()
	 * @brief Check if all rows in this frame are the same size.
	 * @returns bool	- ( true = frame is valid ) ( false = frame is invalid, row size is variable. )
	 */
	bool isValidSize()
	{
		if ( !_frame.empty() ) {
			const auto expected_row_length{ _frame.at(0).size() };
			for ( auto& it : _frame )
				if ( it.size() != expected_row_length )
					return false;
			return true;
		}
		return false;
	}
	
	/**
	 * size()
	 * @brief Returns the size of the frame if it is valid.
	 * @returns Coord
	 */
	Coord size()
	{
		if ( !_frame.empty() && isValidSize() )
			return{ static_cast<long>(_frame.at(0).size()), static_cast<long>(_frame.size()) };
		return{ 0,0 };
	}

	/**
	 * draw()
	 * @brief Draws this frame to the console at it's origin point.
	 */
	void draw()
	{
		// use dual-iterators to iterate both the frame, and console position from the origin offset
		for ( int consoleY{ _origin._y }, frameY{ 0 }; consoleY < _origin._y + static_cast<int>(_frame.size()); consoleY++, frameY++ ) {
			for ( int consoleX = _origin._x, frameX{ 0 }; consoleX < _origin._x + static_cast<int>(_frame.at(frameY).size()); consoleX++, frameX++ ) {
				sys::cursorPos(consoleX * 2, consoleY);		// set the cursor position
				printf("%c", _frame.at(frameY).at(frameX));
				if ( _space_columns )
					printf(" ");
			}
		}
	}

	/** STATIC **
	 * buildFromCell(Cell&, Coord)
	 * @brief Static function that builds a frame from a given cell. Similar to Gamespace::buildNextFrame(), but does not have knowledge of actors.
	 *
	 * @param cell			- A cell reference to build from
	 * @param displayOrigin	- A point in the console window, measured in characters, that will act as the frame's top left corner.
	 * @returns Frame		- The cell's worldspace as a frame object.
	 */
	static Frame buildFromCell(Cell& cell, const Coord& displayOrigin = Coord(0, 0))
	{
		std::vector<std::vector<char>> matrix;
		for ( auto y = 0; y < cell._max._y; y++ ) { // iterate Y-axis
			std::vector<char> row;
			for ( auto x = 0; x < cell._max._x; x++ ) { // iterate X-axis
				row.push_back(static_cast<char>(cell.get(x, y)._display));
			}
			matrix.push_back(row);
		}
		return Frame{ matrix, displayOrigin };
	}

	/** STATIC **
	 * buildFromFile(string)
	 * @brief Builds a Frame object from a given file. This function is similar to the importMatrix() function in cell.h, but returns a Frame.
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
					for ( auto& x : line ) {
						row.push_back(x);
					}
					matrix.push_back(row);
					line.clear();
				}
				// fill in any lines that are too short with blanks
				for ( auto& it : matrix ) {
					if ( it.size() < longest_line ) it.push_back(' ');
				}
			}
		}
		return Frame{ matrix };
	}

	// Stream insertion operator
	friend std::ostream &operator<<(std::ostream &os, const Frame &f)
	{
		for ( const auto& y : f._frame ) {
			for ( auto x : y ) {
				os << x << ' ';
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
class FrameBuffer {
	// The initialized flag, this is set to true once the frame has already been drawn to the console.
	bool _initialized{ false };
	// The last frame printed to the console. (Or the currently displayed frame if this frame-buffer is currently running.)
	Frame _last;
	// A reference to the attached gamespace
	Gamespace& _game;
	// The origin-point of this frame, which is the top-left corner of the matrix. Measured in screen buffer characters
	Coord _window_origin, _origin, _size, _playerStatOrigin;

	/**
	 * initConsole()
	 * @brief Changes the size & position of the console window, and hides the cursor.
	 * @returns bool	- ( true = success ) ( false = Failed to retrieve information about the console window. )
	 */
	[[nodiscard]] bool initConsole() const
	{
		// Get size of each character
		TEXTMETRIC tx{};
		if ( GetTextMetrics(GetDC(GetConsoleWindow()), &tx) ) {
			const int ch_width{ tx.tmMaxCharWidth + 5 }, ch_height{ tx.tmHeight + 5 };

			// Get size of the cell
			const auto cellSize{ _game.getCellSize() };

			// move the window, and change its size
			MoveWindow(GetConsoleWindow(), _window_origin._x, _window_origin._y, (cellSize._x + cellSize._x / 4) * ch_width, (cellSize._y + cellSize._y / 4) * ch_height, TRUE);

			// hide the cursor
			sys::cursorVisible(false);

			return true;
		}
		return false;
	}

	/**
	 * initFrame(bool)
	 * @brief Initializes the frame display. Throws std::exception if cell size is empty.
	 * @param doCLS	 - (Default: true) When true, the screen buffer is overwritten with blank spaces before initializing the frame.
	 */
	void initFrame(const bool doCLS = true)
	{
		if ( !_initialized ) {
			if ( _game.getCellSize()._x > 0 && _game.getCellSize()._y > 0 ) {
				if ( doCLS )
					sys::cls();			// Clear the screen before initializing
				_last = buildNextFrame(_origin);	// set the last frame
				_last.draw();		// draw frame
				_initialized = true;	// set init frame boolean
			}
			else throw std::exception("Cannot initialize an empty cell!");
		} // else do nothing
	}

	/**
	 * buildNextFrame(Coord)
	 * @brief Returns a new frame of the entire cell
	 * @param origin	- The top-left corner of the frame, as shown in the console screen buffer
	 * @returns Frame
	 */
	[[nodiscard]] Frame buildNextFrame(const Coord& origin) const
	{
		std::vector<std::vector<char>> buffer; buffer.reserve(_size._y);
		for ( auto y = 0; y < static_cast<signed>(buffer.capacity()); y++ ) {
			std::vector<char> row; row.reserve(_size._x);
			for ( auto x = 0; x < static_cast<signed>(row.capacity()); x++ ) {
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

	/**
	 * playerStatDisplay()
	 * @brief Displays the player statistics bar.
	 * @param showValues	- (Default: false) When true, the health/stamina values are displayed below their respective bar.
	 */
	void playerStatDisplay(const bool showValues = false) const
	{
		sys::colorReset();

		// Get a pointer to the player
		auto* player{ &_game.getPlayer() };
		
		const auto // Get player integer stats
			health{ player->getHealth() },
			maxHealth{ player->getMaxHealth() },
			healthSegment{ maxHealth / 10 },
			stamina{ player->getStamina() },
			maxStamina{ player->getMaxStamina() },
			staminaSegment{ maxStamina / 10 };
		
		const auto HEADER{ player->name() + " Stats Level " + std::to_string(player->getLevel()) };

		// { (((each box length) + (4 for bar edges & padding)) * (number of stat bars)) }
		const auto lineLength = 28;// (10 + 4) * 2;

		sys::cursorPos(lineLength / 2 - static_cast<signed>(HEADER.size()) / 2 + static_cast<int>(_playerStatOrigin._x), static_cast<int>(_playerStatOrigin._y));
		printf("%s", HEADER.c_str());

		// next line
		sys::cursorPos(_playerStatOrigin._x + 1, _playerStatOrigin._y + 1);

		// Print the health bar
		printf("[");
		sys::colorSet(Color::f_red);
		for ( auto i = 1; i <= 10; ++i ) {
			if ( health >= i * healthSegment )
				printf("@");
			else 
				printf(" ");
		}
		
		// Print health/stamina bar buffer
		sys::colorReset();
		printf("]  [");
		sys::colorSet(Color::f_green);
		
		// Print the stamina bar
		for ( auto i = 1; i <= 10; ++i ) {
			if ( stamina >= i * staminaSegment )
				printf("@");
			else 
				printf(" ");
		}
		// Print stamina bar end buffer
		sys::colorReset();
		printf("]");

		if ( showValues ) {
			sys::cursorPos(_playerStatOrigin._x + 1, _playerStatOrigin._y + 2);
			printf("Health: ");
			sys::colorSet(Color::f_red);
			printf("%d", health);
			sys::colorReset();
			if ( health < 10 )	 printf(" ");
			if ( health < 100 )	 printf(" ");
			
			// Print stamina values
			printf("Stamina: ");
			sys::colorSet(Color::f_green);
			printf("%d", stamina);
			sys::colorReset();
			if ( stamina < 10 )	 printf(" ");
			if ( stamina < 100 ) printf(" ");
			// Set the cursor position to the next line
			sys::cursorPos(_playerStatOrigin._x + 28 / 2 - 5, _playerStatOrigin._y + 3);
		}
		else sys::cursorPos(_playerStatOrigin._x + 28 / 2 - 5, _playerStatOrigin._y + 2);

		printf("Kills: ");
		sys::colorSet(Color::f_red);
		printf("%d", player->getKills());
		sys::colorReset();
	}

public:

	/** CONSTRUCTOR **
	 * FrameBuffer(Cell&, Coord&, Coord&)
	 * @brief Instantiate a FrameBuffer display. Throws std::exception if console window did not initialize correctly.
	 * @param gamespace		- Reference to the attached Gamespace instance
	 * @param origin		- Display origin point, measured in chars. This is the top-left corner.
	 * @param windowOrigin	- (Default: (1,1)) Position of the window on the monitor
	 */
	FrameBuffer(Gamespace& gamespace, const Coord& origin, const Coord& windowOrigin = Coord(1, 1)) : _game(gamespace), _window_origin(windowOrigin), _origin(origin), _size(gamespace.getCellSize()), _playerStatOrigin((_origin._x + _size._x) * 2 / 4, _origin._y + _size._y + _size._y / 10)
	{
		if ( !initConsole() ) throw std::exception("The console window failed to initialize.");
	}

	/**
	 * display()
	 * @brief Update the currently drawn frame, will automatically initialize the frame if necessary. This function will also perform the cleanupDead() function each time it is called.
	 */
	void display()
	{
		fflush(stdout);
		// Remove dead actors
		_game.cleanupDead();
		// get a pointer to the game flare
		auto* flare{ _game.getFlare() };
		// Check if the frame is already initialized
		if ( _initialized ) {
			// flush the output buffer to prevent garbage characters from being displayed.
			// Get the new frame
			auto next = buildNextFrame(_origin);
			// iterate vertical axis
			for ( long frameY{ 0 }, consoleY{ _origin._y }; frameY < static_cast<long>(next._frame.size()); frameY++, consoleY++ ) {
				// iterate horizontal axis for each vertical index
				for ( long frameX{ 0 }, consoleX{ _origin._x }; frameX < static_cast<long>(next._frame.at(frameY).size()); frameX++, consoleX++ ) {
					sys::colorReset();
					// check if the tile at this pos is known to the player
					if ( _game.getTile(frameX, frameY)._isKnown ) {
						auto* const actor = _game.getActorAt(frameX, frameY);
						auto* const item = _game.getItemAt(frameX, frameY);
						// Check if an actor is located here
						if ( actor != nullptr ) {
							// set the cursor position to target
							sys::cursorPos(consoleX * 2, consoleY);
							// print the actor
							actor->print();
						}
						else if ( item != nullptr ) {
							// set the cursor position to target
							sys::cursorPos(consoleX * 2, consoleY);
							// print the item
							item->print();
						}
						// Check if the game wants a screen color flare
						else if ( flare != nullptr && flare->pattern(frameX, frameY) ) {
							// set the cursor position to target. (frameX is multiplied by 2 because every other column is blank space)
							sys::cursorPos(consoleX * 2, consoleY);
							if ( flare->time() % 2 == 0 && flare->time() != 1 ) {
								sys::colorSet(flare->color());
								printf("%c", next._frame.at(frameY).at(frameX));
								sys::colorReset();
							}
							else
								printf("%c", next._frame.at(frameY).at(frameX));
						}
						// Actor is not located here, show the tile char.
						else if ( next._frame.at(frameY).at(frameX) != _last._frame.at(frameY).at(frameX) ) {
							// set the cursor position to target. (frameX is multiplied by 2 because every other column is blank space)
							sys::cursorPos(consoleX * 2, consoleY);
							printf("%c", next._frame.at(frameY).at(frameX));
						} // else tile has not changed, do nothing
					}
					// if the tile is not known, show a blank space instead.
					else {
						// set the cursor position to target
						sys::cursorPos(consoleX * 2, consoleY);
						// output blank
						printf(" ");
					}
				}
			}
			// set the last frame to this frame.
			_last = next;
		}
		else initFrame();
		// display the player stat bar
		playerStatDisplay();
		// do flare functions
		if ( flare != nullptr ) {
			if ( flare->time() > 1 ) 
				flare->decrement();
			else 
				_game.resetFlare();
		}
	}

	/**
	 * deinitialize()
	 * @brief Set the initialized flag to false, causing the display to be re-initialized next time display() is called.
	 */
	void deinitialize()
	{
		_initialized = false;
	}
};