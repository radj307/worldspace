/**
 * game.h
 * Represents the game, with a worldspace & actors.
 * Contains the Gamespace class, which is a container for cells, and actors.
 * by radj307
 */
#pragma once
#include "FrameBuffer.h"

struct GameRules {
	unsigned int trap_damage{ 20 };					// the amount of health an actor loses when they step on a trap
	bool trap_damage_is_percentage{ true };			// whether the trap_damage amount is a percentage of the actor's max health
	unsigned int enemy_count{ 10 };					// how many enemies are present when the game starts
	unsigned int npc_move_chance_per_cycle{ 4 };	// 1 in (this) chance of a hostile moving each cycle (range is 0-this)
	bool walls_are_always_visible{ true };
};

class Gamespace {
	// worldspace cell
	Cell _world;
	// Reference to the game's ruleset
	GameRules &_ruleset;
	// Cache of the last displayed frame
	Frame _world_frame_last, _playerStat_frame_last;
	Coord _world_frame_origin{3,3};
	// Randomization engine
	tRand _rng;

	// player character
	Player _player;
	// generic enemies
	std::vector<Enemy> _hostile;

	/**
	 * playerStatDisplay()  
	 * The player statistics bar
	 */
	void playerStatDisplay()
	{
		const std::string HEADER{ "Player Stats" };
	//	const int BAR_LENGTH{ 10 }, PADDING_LENGTH{ 2 };
	//	const char BAR_EDGE_BEG{ '[' }, BAR_EDGE_END{ ']' }, BAR_SEG{ '@' };
		// { (((each box length) + (4 for bar edges & padding)) * (number of stat bars)) }
		int lineLength = 28;// (10 + 4) * 2;

		// calculate the position to display at
		Coord targetDisplayPos{ ((_world_frame_origin._x + _world._sizeH) * 2 / 4), ((_world_frame_origin._y + _world._sizeV) + (_world._sizeV / 10)) };
		sys::setCursorPos(targetDisplayPos._x + ((lineLength / 2) - (HEADER.size() / 2)), targetDisplayPos._y);
		std::cout << HEADER;
		// move to next line
		sys::setCursorPos(targetDisplayPos._x + 1, targetDisplayPos._y + 1);

		std::cout << '[' << termcolor::red;
		int segment{ static_cast<int>(_player._health / 10) };
		for ( int i = 0; i < 10; i ++ ) {
			if ( i < segment && segment != 0 && segment < _player._MAX_HEALTH)	
				std::cout << '@';
			else std::cout << ' ';
		}
		segment = { static_cast<int>(_player._stamina / 10) };
		std::cout << termcolor::reset << "]  [" << termcolor::green;
		for ( int i = 0; i < 10; i++ ) {
			if ( i < segment && segment != 0 && segment < _player._MAX_HEALTH )
				std::cout << '@';
			else std::cout << ' ';
		}
		std::cout << termcolor::reset << ']';
		int lastCheck{ 100 };
		for ( auto it = _hostile.begin(); it != _hostile.end(); it++ ) {
			int dist = _player._getDist(it->_myPos);
			if ( dist < lastCheck )
				lastCheck = dist;
		}
		sys::setCursorPos(targetDisplayPos._x + 1, targetDisplayPos._y + 2);
		std::cout << "Distance to nearest hostile: " << lastCheck;
		if ( lastCheck < 10 )
			std::cout << ' ';
		if ( lastCheck < 100 )
			std::cout << ' ';
	}

	/**
	 * populateHostileVec(const int)
	 * Create a number of hostiles with random attributes
	 * 
	 * @param count	- The number of hostiles to generate
	 */
	void populateHostileVec(const int count)
	{
		int i = 0;
		for ( Coord pos{ _rng.get(_world._sizeH - 1, 1), _rng.get(_world._sizeV - 1, 1) }; i < count; pos = Coord(_rng.get(_world._sizeH - 1, 1), _rng.get(_world._sizeV - 1, 1)) ) {
			if ( _world.get(pos)._canMove && !_world.get(pos)._isTrap ) {
				_hostile.push_back(Enemy(("Enemy " + std::to_string(i)), pos, 'Y', ActorBase::color::red));
				i++;
			}
		}
	}

	/**
	 * cleanupDead()
	 * Remove dead actors from the game.
	 */
	inline void cleanupDead()
	{
		for ( size_t it = 0; it < _hostile.size(); it++ )
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

	/**
	 * getDirTo(Coord, Coord)
	 * Returns a direction char from a start point and end point.
	 * 
	 * @param pos		- The starting/current position
	 * @param target	- The target position
	 * @returns char	- w = up/s = down/a = left/d = right
	 */
	inline char getDirTo(Coord pos, Coord target)
	{
		int distX{ pos._x - target._x }, distY{ pos._y - target._y };
		// reduce large X distances to -1 or 1
		if ( distX < -1 )		distX = -1;
		else if ( distX > 1 )	distX = 1;
		// reduce large Y distances to -1 or 1
		if ( distY < -1 )		distY = -1;
		else if ( distY > 1 )	distY = 1;
		// select a direction
		if ( distX == 0 )		return { (distY < 0) ? ('s') : ('w') }; // check if X-axis is aligned
		else if ( distY == 0 )	return { (distX < 0) ? ('d') : ('a') }; // check if Y-axis is aligned
		else { // neither axis is aligned, select a random direction
			switch ( _rng.get(1, 0) ) {
			case 0: // move on Y-axis
				return { (distY < 0) ? ('s') : ('w') };
				break;
			case 1: // move on X-axis
				return { (distX < 0) ? ('d') : ('a') };
				break;
			default:return(' ');
			}
		}
	}

	/**
	 * canMove(Coord)
	 * Returns true if the target position can be moved to.
	 *
	 * @param pos	 - Target position
	 * @returns bool - ( false = cannot move ) ( true = can move )
	 */
	inline bool canMove(Coord pos)
	{
		return ((_world.get(pos)._canMove && getActorAt(pos) == nullptr) ? (true) : (false));
	}
	/**
	 * canMove(int, int)
	 * Returns true if the target position can be moved to.
	 * 
	 * @param posX	 - Target X position
	 * @param posY	 - Target Y position
	 * @returns bool - ( false = cannot move ) ( true = can move )
	 */
	inline bool canMove(int posX, int posY)
	{
		return ((_world.get(posX, posY)._canMove && getActorAt(posX, posY) == nullptr) ? (true) : (false));
	}

	/**
	 * attack(ActorBase*, ActorBase*)
	 * Allows an actor to attack another actor.
	 * 
	 * @param attacker	- A pointer to the attacking actor
	 * @param target	- A pointer to the actor being attacked
	 * @returns int		- ( -1 = nullptr was passed, OR stamina is empty ) ( 0 = success, target is still alive ) ( 1 = success, target is dead )
	 */
	inline int attack(ActorBase* attacker, ActorBase* target)
	{
		if ( (attacker != nullptr && target != nullptr)/*&& !(!attacker->_isPlayer && !target->_isPlayer)*/ ) {

			int dmg = _rng.get(attacker->_MAX_DAMAGE, attacker->_MAX_DAMAGE / 5);
			target->_health -= dmg;
			attacker->_stamina -= dmg / 4;
			// if attacker is out of stamina, they receive a quarter of the damage
			if ( attacker->_stamina < dmg / 4 )
				attacker->_health -= dmg / 4;
			if ( target->_health <= 0 || target->_health > target->_MAX_HEALTH ) {
				target->_dead = true;
				return 1;
			}
			return 0;
		}
		return -1;
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
			bool didMove{ false }; // boolean to 
			ActorBase* target{ nullptr }; // declare a pointer to potential attack target
			int attackCode{ 0 }; // integer to hold the return val of potential attack
			switch ( dir ) {
			case 'W':
			case 'w':
				target = getActorAt(actor->_myPos._x, actor->_myPos._y - 1); // define the target
				if ( target != nullptr )
					attackCode = attack(actor, target);
				else if ( attackCode == 1 || canMove(actor->_myPos._x, actor->_myPos._y - 1) ) {
					actor->moveU();
					didMove = true;
				}
				break;
			case 'S':
			case 's':
				target = getActorAt(actor->_myPos._x, actor->_myPos._y + 1); // define the target
				if ( target != nullptr )
					attackCode = attack(actor, target);
				else if ( attackCode == 1 || canMove(actor->_myPos._x, actor->_myPos._y + 1) ) {
					actor->moveD();
					didMove = true;
				}
				break;
			case 'A':
			case 'a':
				target = getActorAt(actor->_myPos._x - 1, actor->_myPos._y); // define the target
				if ( target != nullptr )
					attackCode = attack(actor, target);
				else if ( attackCode == 1 || canMove(actor->_myPos._x - 1, actor->_myPos._y) ) {
					actor->moveL();
					didMove = true;
				}
				break;
			case 'D':
			case 'd':
				target = getActorAt(actor->_myPos._x + 1, actor->_myPos._y); // define the target
				if ( target != nullptr )
					attackCode = attack(actor, target);
				else if ( attackCode == 1 || canMove(actor->_myPos._x + 1, actor->_myPos._y) ) {
					actor->moveR();
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

public:
	// Has the frame already been printed to the console?
	bool _initFrame{ false };

	Gamespace(GLOBAL& settings, GameRules &ruleset) : _ruleset(ruleset), _world((settings._import_filename.size() > 0) ? (Cell{ settings._import_filename, ruleset.walls_are_always_visible, settings._override_known_tiles }) : (Cell{ settings._cellSize, ruleset.walls_are_always_visible, settings._override_known_tiles })), _player("Player", Coord(1, 1), '$', ActorBase::color::green, 3)
	{
		_world.modVis(true, _player._myPos, _player._discoveryRange); // allow the player to see the area around them before initializing
		populateHostileVec(ruleset.enemy_count); // create an amount of enemies specified by the ruleset
		
		// modify console window
		Coord windowSize(_world._sizeH + (_world._sizeH / 4), _world._sizeV + (_world._sizeV / 4));

		//HWND hwnd = GetConsoleWindow();
		TEXTMETRIC tx{};
		GetTextMetrics(GetDC(GetConsoleWindow()), &tx);
		
		int ch_width{ tx.tmMaxCharWidth + 5 }, ch_height{ tx.tmHeight + 5 };
		
		// move the window, and change its size
		MoveWindow(GetConsoleWindow(), 10, 10, ((_world._sizeH + (_world._sizeH / 4)) * (ch_width)), ((_world._sizeV + (_world._sizeV / 4)) * (ch_height)), TRUE);

		// hide the cursor
		sys::hideCursor();
	}

	/**
	 * actionHostile()
	 * Iterates the list of enemies and makes them perform a random action.
	 */
	void actionHostile()
	{
		for ( auto it = _hostile.begin(); it != _hostile.end(); it++ ) {
			// if player is nearby
			if ( (_player._getDist((&*it)->_myPos) < 3) || (&*it)->_aggro ) {
				if ( (&*it)->_aggro == 0 ) {
					(&*it)->_aggro = 6;
					move(&*it, getDirTo((&*it)->_myPos, _player._myPos));
				}
				else if ( (&*it)->_aggro ) {
					(&*it)->_aggro--;
					move(&*it, getDirTo((&*it)->_myPos, _player._myPos));
				}
			}
			// else 1 in 3 chance of a hostile moving each cycle
			else if ( _rng.get(_ruleset.npc_move_chance_per_cycle, 0u) == 0u ) {
				// check if this actor is dead
				if ( ((&*it)->_health == 0 || (&*it)->_health > (&*it)->_MAX_HEALTH) )
					(&*it)->_dead = true;
				else // actor is alive, perform an action
					move(&*it, intToDir(_rng.get(3, 0)));
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
	int actionPlayer(char key)
	{
		// if move was successful
		if ( move(&_player, key) ) {
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
	 * initFrame()
	 * Initializes the frame display.
	 * Throws std::exception if cell size is empty.
	 */
	void initFrame(Coord origin)
	{
		if ( !_initFrame ) {
			if ( _world._sizeH > 0 && _world._sizeV > 0 ) {
				sys::cls();			// Clear the screen before initializing
				_world_frame_last = getFrame(origin);	// set the last frame
				_world_frame_last.draw();		// draw frame
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
			Frame next = getFrame(_world_frame_origin);
			for ( long frameY{ 0 }, consoleY{ _world_frame_origin._y }; frameY < static_cast<long>(next._frame.size()); frameY++, consoleY++ ) {
				for ( long frameX{ 0 }, consoleX{ _world_frame_origin._x }; frameX < static_cast<long>(next._frame.at(frameY).size()); frameX++, consoleX++ ) {
					if ( _world.get(frameX, frameY)._isKnown ) {
						ActorBase* ptr = getActorAt(Coord(frameX, frameY)); // set a pointer to actor at this pos if they exist
						if ( ptr != nullptr ) {
							// set the cursor position to target
							sys::setCursorPos((consoleX * 2), consoleY);
							// output actor with their color
							std::cout << *ptr << ' ';
						}
						else if ( next._frame.at(frameY).at(frameX) != _world_frame_last._frame.at(frameY).at(frameX) ) {
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
			_world_frame_last = next;
		}
		else initFrame(_world_frame_origin);
		// show the player stat frame
		playerStatDisplay();
	}
};