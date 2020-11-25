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

	/** CONSTRUCTOR
	 * Frame()  
	 * Instantiate a blank frame.
	 */
	Frame() : _frame() {}

	/** CONSTRUCTOR
	 * Frame(vector<vector<char>>)
	 * Instantiate a pre-made frame
	 */
	Frame(std::vector<std::vector<char>> coutFrame) : _frame(coutFrame) {}

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

	/**
	 * draw()
	 * Draws this frame to the console at it's origin point.
	 */
	inline void draw()
	{
		int lineCount{ 0 };
		for ( auto y = _frame.begin(); y != _frame.end(); y++ ) {
			for ( auto x = (*y).begin(); x != (*y).end(); x++ ) {
				std::cout << (*x) << ' ';
			}
			lineCount++;
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

struct GameRules {
	unsigned int trap_damage{ 20 };			// the amount of health an actor loses when they step on a trap
	bool trap_damage_is_percentage{ true };	// whether the trap_damage amount is a percentage of the actor's max health

};

class Gamespace {
	// Has the frame already been printed to the console?
	bool _initFrame{ false };
	// Cache of the last displayed frame
	Frame _last;

	// actors
	Player _player;
	std::vector<Enemy> _hostile;

	/**
	 * playerStatDisplay()  
	 * The player statistics bar
	 */
	void playerStatDisplay()
	{
		std::string header = "Player Stats";
		// { (((each box length) + (4 for bar edges & padding)) * (number of stat bars)) }
		const int lineLength = 28;// (10 + 4) * 2;
		Coord targetDisplayPos{ Coord(((_world._sizeH * 2) / 4) - 1, _world._sizeV + (_world._sizeV / 10)) };
		sys::setCursorPos(targetDisplayPos._x + ((lineLength / 2) - (header.size() / 2)), targetDisplayPos._y);
		std::cout << header;
		// move to next line
		sys::setCursorPos(targetDisplayPos._x + 1, targetDisplayPos._y + 1);

		std::cout << '[' << termcolor::red;
		for ( unsigned int i = 0; i < 10; i ++ ) {
			if ( i < (_player._health / 10) )	std::cout << '@';
			else								std::cout << ' ';
		}
		std::cout << termcolor::reset << "]  [" << termcolor::green;
		for ( unsigned int i = 0; i < 10; i++ ) {
			if ( i < (_player._stamina / 10) )	std::cout << '@';
			else								std::cout << ' ';
		}
		std::cout << termcolor::reset << ']';
	}

	/**
	 * populateHostileVec(const int)
	 * Create a number of hostiles with random attributes
	 * 
	 * @param count	- The number of hostiles to generate
	 */
	void populateHostileVec(const int count)
	{
		tRand rng;
		int i = 0;
		for ( Coord pos{ rng.get(_world._sizeH - 1, 1), rng.get(_world._sizeV - 1, 1) }; _world.get(pos)._canMove; pos = Coord(rng.get(_world._sizeH - 1, 1), rng.get(_world._sizeV - 1, 1)) ) {
			_hostile.push_back(Enemy(Coord(rng.get(_world._sizeH - 1, 1), rng.get(_world._sizeV - 1, 1)), 'Y'));
			i++;
			if ( i >= count )
				break;
		}
	}

public:
	// worldspace cell
	Cell &_world;
	GameRules &_ruleset;
	// player position ptr
	Coord *_playerPos{ nullptr };

	Gamespace(Cell &worldspace, GameRules &ruleset, Coord resolution) : _world(worldspace), _ruleset(ruleset), _player(Coord(1, 1), '$', 3), _playerPos(&_player._myPos)
	{
		//_world.discover(_player._myPos, _player._discoveryRange);
		// create some enemies
		populateHostileVec(10);
		Coord windowSize(_world._sizeH + (_world._sizeH / 4), _world._sizeV + (_world._sizeV / 4));
		// move the window, and change its size
		MoveWindow(GetConsoleWindow(), 10, 10, 800, 600, TRUE);
		// hide the cursor
		sys::hideCursor();
	}

	/**
	 * move(ActorBase*, char)
	 * Attempts to move the target actor to an adjacent tile, and processes trap logic.
	 * 
	 * @param actor	- A pointer to the target actor
	 * @param dir	- (w = up / s = down / a = left / d = right) all other characters are ignored.
	 */
	inline bool move(ActorBase* actor, char dir)
	{
		bool rc{ false };
		Tile *tile{ nullptr };
		switch ( dir ) {
		case 'W':
		case 'w':
			tile = &_world.get(actor->_myPos._x, actor->_myPos._y - 1);
			if ( (*tile != __TILE_ERROR) && (tile->_canMove) ) {
				actor->_myPos._y--;
				rc = true;
			}
			break;
		case 'S':
		case 's':
			tile = &_world.get(actor->_myPos._x, actor->_myPos._y + 1);
			if ( (*tile != __TILE_ERROR) && (tile->_canMove) ) {
				actor->_myPos._y++;
				rc = true;
			}
			break;
		case 'A':
		case 'a':
			tile = &_world.get(actor->_myPos._x - 1, actor->_myPos._y);
			if ( (*tile != __TILE_ERROR) && (tile->_canMove) ) {
				actor->_myPos._x--;
				rc = true;
			}
			break;
		case 'D':
		case 'd':
			tile = &_world.get(actor->_myPos._x + 1, actor->_myPos._y);
			if ( (*tile != __TILE_ERROR) && (tile->_canMove) ) {
				actor->_myPos._x++;
				rc = true;
			}
			break;
		default:return false; // if the given char does not match a direction, return to avoid processing the trap logic twice.
		}
		// if this tile is a trap
		if ( _world.get(actor->_myPos)._isTrap ) {
			switch ( _ruleset.trap_damage_is_percentage ) {
			case true:
				actor->_health -= (actor->_MAX_HEALTH * (static_cast<float>(_ruleset.trap_damage) / 100.0f));
				break;
			default:
				actor->_health -= _ruleset.trap_damage;
				break;
			}
		}
		return rc;
	}

	/**
	 * hostileTurn()
	 * Iterates the list of enemies and makes them perform a random action.
	 */
	void hostileTurn()
	{
		tRand rng;
		// iterate the hostile vectors
		for ( auto it = _hostile.begin(); it != _hostile.end(); it++ ) {
			ActorBase* ptr = &*it;
			switch ( rng.get(50u, 0u) ) {
			case 0:
			case 1:
				move(&*it, 'w'); // move up
				break;
			case 2:
			case 3:
				move(&*it, 's'); // move down
				break;
			case 4:
			case 5:
				move(&*it, 'a'); // move left
				break;
			case 6:
			case 7:
				move(&*it, 'd'); // move right
				break;
			default:break;
			}
		}
	}

	/**
	 * movePlayer(char)
	 * Moves the player in a given direction, if possible.
	 * 
	 * @param dir	- 'w' for up, 's' for down, 'a' for left, 'd' for right. Anything else is ignored.
	 */
	void movePlayer(char dir)
	{
		// if move was successful
		if ( move(&_player, dir) ) {
			// player specific post-movement functions
			_world.discover(_player._myPos, _player._discoveryRange);
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
	 * getFrame()
	 * Returns a new frame of the entire cell
	 * 
	 * @returns Frame
	 */
	inline Frame getFrame()
	{
		std::vector<std::vector<char>> buffer;
		for ( int y = 0; y < _world._sizeV; y++ ) {
			std::vector<char> row;
			for ( int x = 0; x < _world._sizeH; x++ ) {
				Coord pos{ Coord(x, y) };
				if ( _world.get(pos)._isKnown ) {
					ActorBase *ptr{ getActorAt(pos) };
					if ( ptr != nullptr ) { // actor exists at position
						row.push_back(char(ptr->_display_char));
					}
					else {
						row.push_back(char(_world.get(pos)._display));
					}
				}
				else row.push_back(char(' '));
			}
			buffer.push_back(row);
		}
		return{ buffer };
	}

	/**
	 * initFrame()
	 * Initializes the frame display
	 */
	void initFrame()
	{
		if ( !_initFrame ) {
			if ( _world._sizeH > 0 && _world._sizeV > 0 ) {
				sys::cls();
				_last = getFrame();	// set the last frame
				_last.draw();		// draw frame
				_initFrame = true;	// set init frame boolean
			}
			else {
				sys::msg(sys::error, "CELL IS EMPTY", "Press any key to exit...");
				exit(1);
			}
		}
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
					if ( _world.get(x, y)._isKnown ) {
						ActorBase* ptr = getActorAt(Coord(x, y)); // set a pointer to actor at this pos if they exist
						if ( ptr != nullptr ) {
							sys::setCursorPos((x * 2), y);
							switch ( ptr->_isPlayer ) {
							case true:
								std::cout << termcolor::green << next._frame.at(y).at(x) << ' ' << termcolor::reset;
								break;
							default:
								std::cout << termcolor::red << next._frame.at(y).at(x) << ' ' << termcolor::reset;
								break;
							}
						}
						else {
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
					else {
						sys::setCursorPos((x * 2), y);
						std::cout << "  ";
					}
				}
			}
			// set the last frame to this frame.
			_last = next;
		}
		else sys::pause("ERROR: EXIT PROGRAM! The frame was not initialized");
		playerStatDisplay();
	}
};