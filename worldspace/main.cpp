/**
 * Project "worldspace"
 * Wrapper for a matrix of tiles that can be used for simple games. Contains built-in console output functions, but can be used as a backend for graphical applications.
 * ~ Evolved into a basic real-time game application utilizing multi-threading.
 * by radj307
 * 
 * 
 *			[File Inheritance Structure]
 * 
 *			   <Coord.h>
 *			   v	  v
 *		 <actor.h>   <cell.h>  <settings.h>
 *			   v      v          v
 *			   <game.h>  <interpret.h>
 *				    v      v
 *				  <threaded.h>
 *					   v
 *				   <main.cpp>
 * 
 * 
 *			[Task List]
	TODO:
		improve the stat display bars
		implement enemy aggression & follow-logic
		player feedback for events
			dead enemies change the color of the tile they died on for a few frames?
			add a text feedback area to the stat display?
			add another stat display box for text feedback?
		notification area for general notifications, that are removed after x seconds

	BACKBURNER:
		implement items
 */
#include "threaded.h"

// commandline to set window size:
// -resolution 800x600
// commandline to load cell from file:
// -file 20x20.txt
int main(int argc, char* argv[])
{
	// run game
	process_game_over(game(argc, argv));

	// return 0 for successful execution
	return 0;
}