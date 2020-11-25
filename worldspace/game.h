/**
 * game.h
 * Represents the game, with a worldspace & actors.
 * Contains the Gamespace class, which is a container for cells, and actors.
 * by radj307
 */
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
	Frame() : _frame(), _origin({0,0}) {}

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

	/**
	 * draw()
	 * Draws this frame to the console at it's origin point.
	 */
	inline void draw()
	{
		for ( int y = _origin._y; y < (_origin._y + (signed)_frame.size()); y++ ) {
			for ( int x = _origin._x; x < (_origin._x + (signed)_frame.at(y).size()); x++ ) {
				sys::setCursorPos((x * 2), y);
				std::cout << _frame.at(y).at(x) << ' ';
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

struct GameRules {
	unsigned int trap_damage{ 20 };			// the amount of health an actor loses when they step on a trap
	bool trap_damage_is_percentage{ true };	// whether the trap_damage amount is a percentage of the actor's max health
	unsigned int enemy_count{ 10 };			// how many enemies are present when the game starts
};

class Gamespace {
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
		//sys::setCursorPos(targetDisplayPos._x + 1, targetDisplayPos._y + 2);
		//std::cout << "Distance to origin: " << _player._getDist(Coord(0, 0));
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
		for ( Coord pos{ rng.get(_world._sizeH - 1, 1), rng.get(_world._sizeV - 1, 1) }; i < count; pos = Coord(rng.get(_world._sizeH - 1, 1), rng.get(_world._sizeV - 1, 1)) ) {
			if ( _world.get(pos)._canMove && !_world.get(pos)._isTrap ) {
				_hostile.push_back(Enemy(("Enemy " + std::to_string(i)), pos, 'Y', ActorBase::color::red));
				i++;
			}
		}
	}

	/**
	 * cleanupDead()
	 * Remove dead NPCs from the game.
	 */
	inline void cleanupDead()
	{
		for ( int it = 0; it < _hostile.size(); it++ )
			if ( _hostile.at(it)._dead )
				_hostile.erase(_hostile.begin() + it);
	}

	/**
	 * intToDir(int)
	 * Converts an integer to a direction char. Used by NPCs to select a movement direction consistent with the player controls.
	 * 
	 * @param i			- ( 0 = 'w'/up ) ( 1 = 's'/down ) ( 2 = 'a'/left ) ( 3 = 'd'/right ) ( other = ' ' )
	 * @returns char	- w/a/s/d
	 */
	inline char intToDir(int i)
	{
		switch ( i ) {
		case 0:return 'w';
		case 1:return 's';
		case 2:return 'a';
		case 3:return 'd';
		default:return ' ';
		}
	}

	// returns true if tile can be moved to
	inline bool canMove(Coord pos)
	{
		return ((_world.get(pos)._canMove && getActorAt(pos) == nullptr) ? (true) : (false));
	}
	// returns true if tile can be moved to
	inline bool canMove(int posX, int posY)
	{
		return ((_world.get(posX, posY)._canMove && getActorAt(posX, posY) == nullptr) ? (true) : (false));
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
		if ( actor != nullptr ) {
			bool didMove{ false };
			switch ( dir ) {
			case 'W':
			case 'w':
				if ( canMove(actor->_myPos._x, actor->_myPos._y - 1) ) {
					actor->moveU();// _myPos._y--;
					didMove = true;
				}
				break;
			case 'S':
			case 's':
				if ( canMove(actor->_myPos._x, actor->_myPos._y + 1) ) {
					actor->moveD();// ._y++;
					didMove = true;
				}
				break;
			case 'A':
			case 'a':
				if ( canMove(actor->_myPos._x - 1, actor->_myPos._y) ) {
					actor->moveL();// ._x--;
					didMove = true;
				}
				break;
			case 'D':
			case 'd':
				if ( canMove(actor->_myPos._x + 1, actor->_myPos._y) ) {
					actor->moveR();// ._x++;
					didMove = true;
				}
				break;
			default:return false; // if the given char does not match a direction, return to avoid processing the trap logic twice.
			}
			// if this tile is a trap
			if ( _world.get(actor->_myPos)._isTrap ) {
				switch ( _ruleset.trap_damage_is_percentage ) {
				case true:
					actor->_health -= static_cast<unsigned int>(static_cast<float>(actor->_MAX_HEALTH) * (static_cast<float>(_ruleset.trap_damage) / 100.0f));
					break;
				default:
					actor->_health -= _ruleset.trap_damage;
					break;
				}
			}
			// check if dead
			if ( actor->_health == 0 || actor->_health > actor->_MAX_HEALTH )
				actor->_dead = true;
			return didMove;
		}
		else throw std::exception("Cannot move() a nullptr!");
	}

public:
	// Has the frame already been printed to the console?
	bool _initFrame{ false };
	// worldspace cell
	Cell _world;
	GameRules &_ruleset;
	// player position ptr
	Coord *_playerPos{ nullptr };

	Gamespace(GLOBAL& settings, GameRules &ruleset) : _world((settings._import_filename.size() > 0) ? (Cell{ settings._import_filename, settings._override_known_tiles }) : (Cell{ settings._cellSize, settings._override_known_tiles })), _ruleset(ruleset), _player("Player", Coord(1, 1), '$', ActorBase::color::green, 3), _playerPos(&_player._myPos)
	{
		_world.modVis(true, _player._myPos, _player._discoveryRange); // allow the player to see the area around them before initializing
		populateHostileVec(ruleset.enemy_count); // create an amount of enemies specified by the ruleset
		Coord windowSize(_world._sizeH + (_world._sizeH / 4), _world._sizeV + (_world._sizeV / 4));
		// move the window, and change its size
		MoveWindow(GetConsoleWindow(), 10, 10, settings._resolution._x, settings._resolution._y, TRUE);
		// hide the cursor
		sys::hideCursor();
	}

	/**
	 * actionHostile()
	 * Iterates the list of enemies and makes them perform a random action.
	 */
	void actionHostile()
	{
		tRand rng;
		// iterate the hostile vectors
		for ( auto it = _hostile.begin(); it != _hostile.end(); it++ ) {
			if ( rng.get(2u, 0u) == 0u ) { // 1 in 3 chance of a hostile moving each cycle
				//ActorBase* ptr = &*it;
				// check if this actor is dead
				if ( ((&*it)->_health == 0 || (&*it)->_health > (&*it)->_MAX_HEALTH) ) {
					(&*it)->_dead = true;
					sys::setCursorPos(0, _world._sizeV + 6);
					sys::msg(sys::log, "");
				}
				else { // actor is alive, perform an action
					move(&*it, intToDir(rng.get(3, 0)));
				}
			}
		}
	}

	/**
	 * actionPlayer(char)
	 * Moves the player in a given direction, if possible.
	 * 
	 * @param dir	- 'w' for up, 's' for down, 'a' for left, 'd' for right. Anything else is ignored.
	 * @returns int	- ( 0 = normal execution ) ( 1 = player is dead )
	 */
	int actionPlayer(char dir)
	{
		// if move was successful
		if ( move(&_player, dir) ) {
			// player specific post-movement functions
			_world.modVis(true, _player._myPos, _player._discoveryRange); // allow the player to see the area around them
			// check if dead
			if ( _player._dead )
				return 1;
		}
		return 0;
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
	ActorBase* getActorAt(int posX, int posY, const bool findByIndex = true)
	{
		if ( posX == _player._myPos._x && posY == _player._myPos._y )
			return &_player; // else:
		for ( auto it = _hostile.begin(); it != _hostile.end(); it++ ) {
			if ( posX == (*it)._myPos._x && posY == (*it)._myPos._y )
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
					if ( ptr != nullptr ) // actor exists at position
						row.push_back(char(ptr->_display_char));
					else
						row.push_back(char(_world.get(pos)._display));
				}
				else row.push_back(char(' '));
			}
			buffer.push_back(row);
		}
		return{ buffer };
	}

	/**
	 * initFrame()
	 * Initializes the frame display.
	 * Throws std::exception if cell size is empty.
	 */
	void initFrame()
	{
		if ( !_initFrame ) {
			if ( _world._sizeH > 0 && _world._sizeV > 0 ) {
				sys::cls();			// Clear the screen before initializing
				_last = getFrame();	// set the last frame
				_last.draw();		// draw frame
				_initFrame = true;	// set init frame boolean
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
		cleanupDead(); // clean up & remove dead actors
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
							// set the cursor position to target
							sys::setCursorPos((x * 2), y);
							// output actor with their color
							std::cout << *ptr << ' ';
						}
						else if ( next._frame.at(y).at(x) != _last._frame.at(y).at(x) ) {
							// set the cursor position to target. (x is multiplied by 2 because every other column is blank space)
							sys::setCursorPos((x * 2), y);
							// print next frame's character to position, followed by a blank space.
							std::cout << next._frame.at(y).at(x) << ' ';
						} // else tile has not changed, do nothing
					}
					else {
						// set the cursor position to target
						sys::setCursorPos((x * 2), y);
						// output blank
						std::cout << "  ";
					}
				}
			}
			// set the last frame to this frame.
			_last = next;
		}
		else initFrame();//sys::pause("ERROR: EXIT PROGRAM! The frame was not initialized");
		playerStatDisplay();
	}
};