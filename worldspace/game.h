/**
 * game.h
 * Represents the game, with a worldspace & actors.
 * Contains the Gamespace class, which is a container for cells, and actors.
 * by radj307
 */
#pragma once
#include "cell.h"
#include "actor.h"
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
	checkBounds *_isValidPos{ nullptr };

	/** CONSTRUCTOR
	 * Frame()  
	 * Instantiate a blank frame.
	 */
	Frame() : _frame() {}

	/** CONSTRUCTOR
	 * Frame(vector<vector<char>>)
	 * Instantiate a pre-made frame
	 */
	Frame(std::vector<std::vector<char>> coutFrame) : _frame(coutFrame), _isValidPos((coutFrame.size() > 0) ? (new checkBounds(coutFrame.at(0).size(), coutFrame.size())) : ( nullptr )) {}

	~Frame()
	{
		if ( _isValidPos != nullptr )
			delete _isValidPos;
	}

	/**
	 * draw()
	 * Draws this frame to the console.
	 */
	inline void draw()
	{
		for ( auto y = _frame.begin(); y != _frame.end(); y++ ) {
			for ( auto x = (*y).begin(); x != (*y).end(); x++ ) {
				std::cout << (*x) << ' ';
			}
			std::cout << std::endl;
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

class Gamespace {
	// Has the frame already been printed to the console?
	bool _initFrame{ false };
	// Cache of the last displayed frame
	Frame _last;

	// actors
	Player _player;
	std::vector<Enemy> _hostile;

public:
	// worldspace cell
	Cell &_world;

	Gamespace(Cell &worldspace) : _world(worldspace), _player(Coord(1, 1), '$')
	{
		// DEBUG -- MOVE THIS TO A FUNCTION LATER
		_hostile.push_back(Enemy(Coord(2,2),'%'));
		_hostile.push_back(Enemy(Coord(5,5),'%'));

		// init last frame
		_last = getFrame();
		_last.draw();
	}

	/**
	 * movePlayer(char)
	 * Moves the player in a given direction, if possible.
	 * 
	 * @param dir	- 'w' for up, 's' for down, 'a' for left, 'd' for right. Anything else is ignored.
	 */
	void movePlayer(char dir)
	{
		Tile *tile{ nullptr };
		switch ( dir ) {
		case 'W':
		case 'w':
			tile = &_world.get(_player._myPos._x, _player._myPos._y - 1);
			if ( (*tile != __TILE_ERROR) && (tile->_canMove) )
				_player._myPos._y--;
			break;
		case 'S':
		case 's':
			tile = &_world.get(_player._myPos._x, _player._myPos._y + 1);
			if ( (*tile != __TILE_ERROR) && (tile->_canMove) )
				_player._myPos._y++;
			break;
		case 'A':
		case 'a':
			tile = &_world.get(_player._myPos._x - 1, _player._myPos._y);
			if ( (*tile != __TILE_ERROR) && (tile->_canMove) )
				_player._myPos._x--;
			break;
		case 'D':
		case 'd':
			tile = &_world.get(_player._myPos._x + 1, _player._myPos._y);
			if ( (*tile != __TILE_ERROR) && (tile->_canMove) )
				_player._myPos._x++;
			break;
		default:break;
		}
	}

	/**
	 * getActorAt(Coord, const bool)
	 * Returns a pointer to an actor located at a given tile.
	 *
	 * @param pos			- The target tile
	 * @param findByIndex	- (Default: true) Whether to search the matrix from pos (0,0)=true or (1,1)=false
	 */
	ActorBase* getActorAt(Coord pos, const bool findByIndex = true)
	{
		if ( pos == _player._myPos )
			return &_player; // else:
		for ( auto it = _hostile.begin(); it != _hostile.end(); it++ ) {
			if ( pos == (*it)._myPos )
				return &*it;
		} // else:
		return{ nullptr };
	}

	/**
	 * getFrame(Coord, const int)
	 * Returns a new frame centered on the given position.
	 * 
	 * @param pos	 - The center point
	 * @param radius - The number of tiles in each direction to include.
	 * @returns Frame
	 */
	inline Frame getFrame(Coord pos, const int radius)
	{
		Frame f;
		// iterate vertical
		for ( int y = (signed)(pos._y - radius); y < (signed)(pos._y + radius); y++ ) {
			std::vector<char> row;
			// counter for number of chars added
			int doNewline{ 0 };
			// iterate horizontal
			for ( int x = (signed)(pos._x - radius); x < (signed)(pos._x + radius); x++ ) {
				// check if this pos exists
				if ( _world.isValidPos(x, y) ) {
					Coord pos{ Coord(x, y) };
					ActorBase *ptr{ getActorAt(pos) };
					if ( ptr != nullptr ) { // actor exists at position
						row.push_back(char(ptr->_display_char));
					}
					else {
						row.push_back(char(_world.get(pos)._display));
					}
					doNewline++;
				}
			}
			// check if a newline should be inserted
			if ( doNewline )
				f._frame.push_back(row);
		}
		return f;
	}

	/**
	 * getFrame()
	 * Returns a new frame of the entire cell
	 * 
	 * @returns Frame
	 */
	inline Frame getFrame()
	{
		Frame f;
		for ( int y = 0; y < _world._sizeV; y++ ) {
			std::vector<char> row;
			for ( int x = 0; x < _world._sizeH; x++ ) {
				Coord pos{ Coord(x, y) };
				ActorBase *ptr{ getActorAt(pos) };
				if ( ptr != nullptr ) { // actor exists at position
					row.push_back(char(ptr->_display_char));
				}
				else {
					row.push_back(char(_world.get(pos)._display));
				}
			}
			f._frame.push_back(row);
		}
		return f;
	}

	/**
	 * initFrame()
	 * Initializes the frame display
	 */
	void initFrame()
	{
		_last = getFrame();	// set the last frame
		_last.draw();		// draw frame
		_initFrame = true;	// set init frame boolean
	}

	/**
	 * initFrame(Coord, const int)
	 * Initializes the frame display with a small part of the cell
	 */
	void initFrame(Coord pos, const int radius)
	{
		_last = getFrame(pos, radius);	// set the last frame
		_last.draw();					// draw frame
		_initFrame = true;				// set init frame boolean
	}

	/**
	 * display()
	 * Update the currently drawn frame.
	 * The frame must be initialized with initFrame() first!
	 */
	void display()
	{
		if ( _initFrame ) {
			// flush the output buffer to prevent garbage characters from being displayed.
			std::cout.flush();
			// Get the new frame
			Frame next = getFrame();
			for ( size_t y = 0; y < next._frame.size(); y++ ) {
				for ( size_t x = 0; x < next._frame.at(y).size(); x++ ) {
					try {
						// check if this character has been updated between frames
						if ( next._frame.at(y).at(x) != _last._frame.at(y).at(x) ) {
							// set the cursor position to target. (x is multiplied by 2 because every other column is blank space)
							sys::setCursorPos((x * 2), y);
							// print next frame's character to position, followed by a blank space.
							std::cout << next._frame.at(y).at(x) << ' ';
						} // else continue
					} catch ( std::exception &ex ) { // Catch possible vector subscript exceptions (out of range)
						sys::msg(sys::error, "\"" + std::string(ex.what()) + "\" thrown in display() at position: (" + std::to_string(y) + ", " + std::to_string(x) + ")");
					}
				}
			}
			// set the last frame to this frame.
			_last = next;
		}
		else initFrame();
	}
};