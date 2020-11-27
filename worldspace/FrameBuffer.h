#pragma once
#include "cell.h"
#include "actor.h"
#include "settings.h"
#include "sys.h"
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
			for ( auto x = (*y).begin(); x != (*y).end(); x++ ) {
				X++;
			}
		}
		return{ X, Y };
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
		for ( int y = 0; y < cell._sizeV; y++ ) { // iterate Y-axis
			std::vector<char> row;
			for ( int x = 0; x < cell._sizeH; x++ ) { // iterate X-axis
				row.push_back(static_cast<char>(cell.get(x, y)._display));
			}
			matrix.push_back(row);
		}
		return{ matrix, displayOrigin };
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
				sys::setCursorPos((consoleX * 2), consoleY);		// set the cursor position
				std::cout << _frame.at(frameY).at(frameX);
				if ( spaceColumns )
					std::cout << ' ';	// draw the frame pos to screen
			}
		}
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


struct FrameBuffer {
	bool _initialized{ false };
	Frame _last;
	Coord _origin;
	Cell& _world;
	std::vector<ActorBase*> _actor_list;

	/** CONSTRUCTOR **
	 * FrameBuffer(Cell&, Coord, vector<ActorBase*>)
	 *
	 * @param cell	 - Ref to a cell
	 * @param origin - Display origin point, measured in chars. This is the top-left corner.
	 * @param vec	 - A vector of pointers to actors
	 */
	FrameBuffer(Cell& cell, Coord origin, std::vector<ActorBase*> actors) : _world(cell), _origin(origin), _actor_list(actors) {}

	/**
	 * getActorAt(Coord, const bool)
	 * Returns a pointer to an actor located at a given tile.
	 *
	 * @param pos			- The target tile
	 * @param findByIndex	- (Default: true) Whether to search the matrix from pos (0,0)=true or (1,1)=false
	 */
	ActorBase* getActorAt(Coord pos, const bool findByIndex = true)
	{
		for ( auto it = _actor_list.begin(); it != _actor_list.end(); it++ ) {
			if ( pos == (*it)->_myPos )
				return *it;
		} // else:
		return{ nullptr };
	}
	/**
	 * getActorAt(Coord, const bool)
	 * Returns a pointer to an actor located at a given tile.
	 *
	 * @param pos			- The target tile
	 * @param findByIndex	- (Default: true) Whether to search the matrix from pos (0,0)=true or (1,1)=false
	 */
	ActorBase* getActorAt(int posX, int posY, const bool findByIndex = true)
	{
		for ( auto it = _actor_list.begin(); it != _actor_list.end(); it++ ) {
			if ( posX == (*it)->_myPos._x && posY == (*it)->_myPos._y )
				return *it;
		} // else:
		return{ nullptr };
	}

	/**
	 * getFrame()
	 * Returns a new frame of the entire cell
	 *
	 * @returns Frame
	 */
	inline Frame getFrame(Coord origin)
	{
		std::vector<std::vector<char>> buffer;
		for ( int y = 0; y < _world._sizeV; y++ ) {
			std::vector<char> row;
			for ( int x = 0; x < _world._sizeH; x++ ) {
				Coord pos{ Coord(x, y) };
				if ( _world.get(pos)._isKnown ) {
					ActorBase *ptr{ getActorAt(pos) };
					if ( ptr != nullptr ) // actor exists at position
						row.push_back(char(ptr->_display_char));
					else
						row.push_back(char(_world.get(pos)._display));
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
	 */
	void initFrame(Coord origin)
	{
		if ( !_initialized ) {
			if ( _world._sizeH > 0 && _world._sizeV > 0 ) {
				sys::cls();			// Clear the screen before initializing
				_last = getFrame(origin);	// set the last frame
				_last.draw();		// draw frame
				_initialized = true;	// set init frame boolean
			}
			else throw std::exception("Cannot initialize an empty cell!");
		} // else do nothing
	}

	/**
	 * display()
	 * Update the currently drawn frame.
	 * The frame must be initialized with initFrame() first!
	 */
	void display()
	{
		if ( _initialized ) {
			// flush the output buffer to prevent garbage characters from being displayed.
			std::cout.flush();
			// Get the new frame
			Frame next = getFrame(_origin);
			for ( long frameY{ 0 }, consoleY{ _origin._y }; frameY < static_cast<long>(next._frame.size()); frameY++, consoleY++ ) {
				for ( long frameX{ 0 }, consoleX{ _origin._x }; frameX < static_cast<long>(next._frame.at(frameY).size()); frameX++, consoleX++ ) {
					if ( _world.get(frameX, frameY)._isKnown ) {
						ActorBase* ptr = getActorAt(Coord(frameX, frameY)); // set a pointer to actor at this pos if they exist
						if ( ptr != nullptr ) {
							// set the cursor position to target
							sys::setCursorPos((consoleX * 2), consoleY);
							// output actor with their color
							std::cout << *ptr << ' ';
						}
						else if ( next._frame.at(frameY).at(frameX) != _last._frame.at(frameY).at(frameX) ) {
							// set the cursor position to target. (frameX is multiplied by 2 because every other column is blank space)
							sys::setCursorPos((consoleX * 2), consoleY);
							// print next frame's character to position, followed by a blank space.
							std::cout << next._frame.at(frameY).at(frameX) << ' ';
						} // else tile has not changed, do nothing
					}
					else {
						// set the cursor position to target
						sys::setCursorPos((consoleX * 2), consoleY);
						// output blank
						std::cout << "  ";
					}
				}
			}
			// set the last frame to this frame.
			_last = next;
		}
		else initFrame(_origin);
	}
};
