/**
 * Project "worldspace"
 * Wrapper for a matrix of tiles that can be used for simple games. Contains built-in console output functions, but can be used as a backend for graphical applications.
 * by radj307
 * 
 *	  [File Inheritance Structure]
 *			   <Coord.h>
 *			   v	  v
 *		 <actor.h>   <cell.h>
 *			   v      v
 *			   <game.h>
 */
#include <mutex>
#include "game.h"
#include "interpret.h"

static const int __FRAMETIME_MS = 30;
static const char __BLANK_KEY = ' ';
struct memory {
	char key{ __BLANK_KEY };	// player's last key press
	bool ready{ false };			// ready to receive input when true

	inline void reset_state()
	{
		key = __BLANK_KEY;
		ready = true;
	}
};
memory mem;

void play()
{
	const int timeout = 100;
	for ( char input{}; input != 'q';  ) {
		if ( mem.ready ) {
			mem.key = std::tolower(_getch());
			mem.ready = false;
		}
		else std::this_thread::sleep_for(std::chrono::milliseconds(__FRAMETIME_MS / 3));
	}
}

inline void run_game(Gamespace& game)
{
	typedef std::chrono::high_resolution_clock HRT;
	typedef std::chrono::seconds s;

	// set the ready var to allow player movement
	mem.ready = true;

	game.initFrame(); // initialize the display

	for ( auto timeOfLastHostileTurn = HRT::now();; ) {
		std::cout.flush();
		switch ( mem.key ) {
		case __BLANK_KEY:break;
		case 'q':
			std::cout << std::endl << std::endl;
			sys::msg(sys::log, "Kill request received. Shutting down.");
			return;
		default:
			game.movePlayer(mem.key);
			mem.reset_state();
			break;
		}

		game.display();
		sys::sleep(__FRAMETIME_MS);

		// do enemy turn if it has been at least 1 second since the last one
		if ( (HRT::now() - timeOfLastHostileTurn) > s(1) ) {
			game.hostileTurn();
			timeOfLastHostileTurn = HRT::now();
		}
	}
}
// commandline to load from file:
//-file 20x20.txt
int main(int argc, char* argv[], char* envp[])
{
	/*
	TODO:
		add tile uncovering; discover tiles as they explore
		stop actors from stacking; add a check in the game::move function
		add attack function when 2 actors collide
	*/
	// Interpret commandline arguments
	GLOBAL g{ interpret(argc, argv) };

	//Cell cell(g._import_filename, g._override_known_tiles);
	Cell cell(g._cellSize, false);// g._override_known_tiles);
	GameRules rules;
	rules.trap_damage = 20;
	rules.trap_damage_is_percentage = true;

	Gamespace game(cell, rules, g._resolution);

	// start a second thread to process player input
	std::thread player(play);
	
	run_game(game);

	// rejoin the player input thread
	player.join();

	sys::msg(sys::log, "done");	
	return 0;
}