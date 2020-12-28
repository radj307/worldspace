/**
 * threaded.h
 * This file contains all of the game-operation logic, including all threads & controllers.
 * by radj307
 */
#pragma once
#include <atomic>	// for thread-safe variables
#include <conio.h>
#include <future>	// for capturing thread return values
#include <mutex>	// for mutexes & scoped locks
#include <thread>	// for threads

#include "FrameBuffer.h"
#include "INI.hpp"
#include "shared.h"
#include "sys.h"
#include "termcolor/termcolor.hpp"

namespace game {
	typedef std::chrono::high_resolution_clock CLK;
	namespace _internal {
		/**
		 * process_game_over(int)
		 * @brief Clears the screen & prints an informational game over message to the terminal.
		 * @param winStateCode	- ( -1 = Player Loses ) ( 0 = Player Wins ) ( 1 = Player exited ) ( Other = Undefined )
		 * @param textPos		- Position in the screen buffer to print message.
		 */
		inline void print_game_over(const int winStateCode, const Coord textPos = Coord(9, 3))
		{
			sys::cls(true);
			sys::cursorPos(textPos);
			switch ( winStateCode ) {
			case PLAYER_LOSE_CODE: // player lost the game
				std::cout << termcolor::red << "You lost!" << termcolor::reset;
				break;
			case PLAYER_WIN_CODE: // player won the game
				std::cout << termcolor::green << "You won!" << termcolor::reset;
				break;
			case PLAYER_QUIT_CODE: // player exited the game
				std::cout << termcolor::cyan << "Game Over." << termcolor::reset;
				break;
			case GAME_EXCEPTION_CODE:
			default: // undefined
				std::cout << termcolor::red << "The game exited with an undefined error." << termcolor::reset;
				break;
			}
			std::this_thread::sleep_for(500ms);
		}

		/**
		 * game_thread_player(Gamespace&, GLOBAL&)
		 * @brief Thread function that receives & processes player key presses.
		 * @param mutx	- Shared Mutex
		 * @param mem	- Shared Memory
		 * @param game	- Reference to the associated gamespace, passed with std::ref()
		 */
		inline void thread_player(std::mutex& mutx, memory& mem, Gamespace& game)
		{
			while ( !mem.kill.load() ) {
				// getch waits until key press, no need to sleep this thread.
				if ( _kbhit() ) {
					const auto key{ static_cast<char>(std::tolower(_getch())) };
					//mem.key.store(static_cast<char>(_getch()));

					// if game is not paused
					if ( !mem.pause.load() ) {
						// switch player keypress
						switch ( key ) {
						case 'q': // player pressed the exit game key
							mem.kill_code.store(PLAYER_QUIT_CODE);
							game._game_state._game_is_over.store(true);
							mem.kill.store(true);
							return;
						case 'p': // player pressed the pause game key
							mem.pause.store(true);
							break;
						default: // player pressed a different key, process it
							std::scoped_lock<std::mutex> lock(mutx); // lock the mutex
							game.actionPlayer(key);
							break;
						}
					} // else check if player wants to unpause
					else if ( key == 'p' ) { mem.unpause_game(); }
				}
				else std::this_thread::sleep_for(__FRAMETIME);
			}
		}

		/**
		 * game_thread_npc(Gamespace&)
		 * @brief Thread function that controls the NPC actors.
		 * @param mutx	- Shared Mutex
		 * @param mem	- Shared Memory
		 * @param game	- Reference to the associated gamespace, passed with std::ref()
		 */
		inline void thread_npc(std::mutex& mutx, memory& mem, Gamespace& game)
		{
			// loop until shared memory's kill flag is true
			while ( !mem.kill.load() ) {
				std::this_thread::sleep_for(__NPC_CLOCK);
				if ( !mem.pause.load() ) {
					std::scoped_lock<std::mutex> lock(mutx);	// lock the mutex
					game.actionAllNPC();						// perform NPC actions
				}
				else std::this_thread::sleep_for(1s);
			}
		}

		/**
		 * game_thread_display(Gamespace&)
		 * @brief Thread function that controls the display.
		 * @param mutx	- Shared Mutex
		 * @param mem	- Shared Memory
		 * @param game	- Reference to the associated gamespace, passed with std::ref()
		 * @param cfg	- Game Rules
		 */
		inline void thread_display(std::mutex& mutx, memory& mem, Gamespace& game, GameRules& cfg)
		{
			// Catch possible exceptions from frame buffer constructor
			try {
				// create a frame buffer with the given gamespace ref
				FrameBuffer gameBuffer(game, Coord(3, 3), Coord(1920 / 3, 1080 / 8));
				// Loop until kill flag is true
				for ( auto tLastRegenCycle{ CLK::now() }; !mem.kill.load(); ) {
					if ( !mem.pause.load() ) {
						mem.pause_complete.store(false);
						std::this_thread::sleep_for(__FRAMETIME);		// sleep this thread for half of a clock cycle
						std::cout.flush();									// flush the cout buffer
						std::scoped_lock<std::mutex> lock(mutx);			// lock the mutex
						game.apply_level_ups();								// Apply level ups
						gameBuffer.display();								// display the gamespace

						if ( CLK::now() - tLastRegenCycle >= cfg._regen_timer ) {
							game.apply_passive();							// Apply passive effects every second
							tLastRegenCycle = CLK::now();
						}
						if ( game._game_state._game_is_over.load() ) {
							mem.kill.store(true);							// Send kill code
							if ( game._game_state._playerDead.load() )
								mem.kill_code.store(PLAYER_LOSE_CODE);
							else if ( game._game_state._allEnemiesDead.load() )
								mem.kill_code.store(PLAYER_WIN_CODE);
							break;											// break loop
						}
					}
					else if ( !mem.pause_complete.load() ) {
						gameBuffer.deinitialize();
						mem.pause_game();
						mem.pause_complete.store(true);
					}
					else std::this_thread::sleep_for(__FRAMETIME);
				}
				// Once the kill flag is true, show the game over message and return
				print_game_over(mem.kill_code.load());
			} catch ( std::exception & ex ) {
				sys::msg(sys::error, "Exception thrown in thread_display(): \"" + std::string(ex.what()) + "\"");
				mem.kill.store(true);
			}
		}

		/**
		 * initDefaultINI(string&)
		 * @brief Writes a default INI file with the given filename.
		 * @param filename	- The name of the created file
		 * @returns bool	- ( true = success ) ( false = an error occurred while writing )
		 */
		inline bool initDefaultINI(const std::string& filename)
		{
			// VARIABLES & VALUES
			const sectionMap defMap{
				{
					"controls", {
						{ "key_up",		std::to_string(_CTRL._KEY_UP)	 },
						{ "key_down",	std::to_string(_CTRL._KEY_DOWN)  },
						{ "key_left",	std::to_string(_CTRL._KEY_LEFT)  },
						{ "key_right",	std::to_string(_CTRL._KEY_RIGHT) },
						{ "key_pause",	std::to_string(_CTRL._KEY_PAUSE) },
						{ "key_quit",	std::to_string(_CTRL._KEY_QUIT)  },
					}
				},
				{
					"world", {
						{ "sizeH",					"30"	 },
						{ "sizeV",					"30"	 },
						{ "showAllTiles",			"false"	 },
						{ "showAllWalls",			"true"	 },
						{ "fogOfWar",				"true"	 },
						{ "importFromFile",			""		 },
						{ "trapDamage",				"20"	 },
						{ "trapDamageIsPercentage", "true"   },
					}
				},
				{
					"actors", {
						{ "attackCostStamina",		 "15"	},
						{ "attackBlockChance",		 "35.0" },
						{ "attackMissChanceFull",	 "11.0" },
						{ "attackMissChanceDrained", "35.0" },
						{ "npcMoveChance",			 "6.0"	},
						{ "npcMoveChanceAggro",		 "6.0"	},
						{ "npcVisModAggro",			 "1"	},
						{ "regen_time",				 "2"	},
						{ "regen_health",			 "5"	},
						{ "regen_stamina",			 "10"	},
						{ "levelKillThreshold",		 "2"	},
						{ "levelKillMult",			 "2"	},
						{ "levelRestorePercent",	 "50"	},
					}
				},
				{
					"enemy", {
						{ "count",			"20" },
						{ "aggroDistance",	"3"	 },
						{ "enable_boss", "true" },
						{ "bossDelayedSpawn", "true" },
					}
				},
				{
					"neutral", {
						{ "count",			"12" },
					}
				},
				{
					"player", {
						{ "name",			"Player" },
						{ "health",			""		 },
						{ "stamina",		""		 },
						{ "damage",			""		 },
						{ "godmode",		"false"  },
					}
				},
				{
					"timing", {
						{ "framerate",		"75"	 },
						{ "npc_cycle",		"225"	 },
					}
				},
			},
			// VARIABLE COMMENTS
			defCommentMap{
				{
					"world", {
						{ "sizeH", "Horizontal World Size" },
						{ "sizeV", "Vertical World Size" },
						{ "showAllTiles", "When true, all tiles are always visible" },
						{ "showAllWalls", "When true, all walls are always visible" },
						{ "fogOfWar", "When true, tile discovery is turned off, and only nearby tiles are visible" },
						{ "importFromFile", "When not blank, attempts to load world from this file" },
						{ "trapDamageIsPercentage", "When true, trapDamage is calculated as a percentage of an actors health" },
					}
				},
				{
					"actors", {
						{ "attackBlockChance", "The chance that an actor will block an attack when they have stamina" },
						{ "attackMissChanceDrained", "The chance that an actor will miss an attack when they are out of stamina" },
						{ "npcMoveChance", "Every NPC cycle, there is a 1 in (x) chance of an NPC moving." },
						{ "npcMoveChanceAggro", "Every NPC cycle, there is a 1 in (x) chance of an NPC moving when they have an active target." },
						{ "regen_time", "Every (x) seconds, all actors regen an amount of their health & stamina" },
						{ "levelRestorePercent", "Every time an actor levels up, this percentage of their stats are regenerated instantly." },
					}
				},
				{
					"timing", {
						{ "framerate", "Target/Max frames per second" },
						{ "npc_cycle", "Time between NPC cycles in milliseconds." },
					}
				},
				{
					"enemy", {
						{ "enable_boss", "When true, a boss is spawned at the end of the game" },
						{ "bossDelayedSpawn", "When true, the boss is spawned after the final challenge is completed. Else, it spawns during." },
					}
				},
			};
			// Write to file & return result
			return INI_Defaults{ defMap, defCommentMap }.write(filename);
		}

		/**
		 * initRuleset(INI&)
		 * @brief Initialize the game ruleset from INI file. If INI file is empty, the default GameRules configuration is used instead.
		 * @param cfg	- INI ref
		 * @returns GameRules
		 */
		inline GameRules initRuleset(INI& cfg)
		{
			if ( cfg.contains("world") && cfg.contains("actors") && cfg.contains("player") && cfg.contains("neutral") && cfg.contains("enemy") ) { // if cfg is not empty
				sys::msg(sys::debug, "Using INI GameRules");
				return GameRules{ cfg };
			}
			sys::msg(sys::debug, "Using default GameRules");
			return{}; // else return default GameRules configuration
		}

		/**
		 * initControlSet(INI&)
		 * @brief Initialize the control set from INI file. If INI does not contain a controls section, the default controlset is used instead.
		 * @param cfg	- INI ref
		 * @returns CONTROLS
		 */
		inline CONTROLS initControlSet(INI& cfg)
		{
			// Check if the INI contains a "controls" section.
			if ( cfg.contains("controls") ) {
				sys::msg(sys::debug, "Using INI ControlSet");
				return CONTROLS{ // Initialize controlset from INI
					cfg.get<char>("controls", "key_up", conv::stoc),
					cfg.get<char>("controls", "key_down", conv::stoc),
					cfg.get<char>("controls", "key_left", conv::stoc),
					cfg.get<char>("controls", "key_right", conv::stoc),
					cfg.get<char>("controls", "key_pause", conv::stoc),
					cfg.get<char>("controls", "key_quit", conv::stoc)
				};
			}
			sys::msg(sys::debug, "Using default ControlSet");
			return _CTRL; // else return the default controlset
		}

		/**
		 * initTiming(INI&)
		 * @brief Initialize timing values for the display thread & npc thread. Sets framerate/time & npcCycle time.
		 * @param cfg	- INI ref
		 */
		inline void initTiming(INI& cfg)
		{
			// Framerate & Frametime
			if ( cfg.contains("timing", "framerate") )
				setFramerate(cfg.get<unsigned int>("timing", "framerate", conv::stoui));
			else
				setFramerate(60);
			// NPC Cycle time
			if ( cfg.contains("timing", "npc_cycle") )
				setNPCCycle(static_cast<std::chrono::milliseconds>(cfg.get<unsigned int>("timing", "npc_cycle", conv::stoui)));
			else
				setNPCCycle(225ms);
		}
	}

	/**
	 * game_start(GLOBAL&)
	 * @brief Starts the game threads and returns once the game is over.
	 * @param INI_Files	- String vector containing INI filenames.
	 * @returns bool	- ( true = game exited normally ) ( false = player pressed the quit button )
	 */
	inline bool start(const std::vector<std::string>& INI_Files)
	{
		// read from INI file
		if ( !file::exists("def.ini") )
			_internal::initDefaultINI("def.ini");
		INI cfg(INI_Files);
		cfg.read("def.ini");

		_internal::initTiming(cfg);						// Init Thread times
		auto rules{ _internal::initRuleset(cfg) };		// Init GameRules
		auto controls{ _internal::initControlSet(cfg) };// Init Controls
		
		__controlset = &controls; // Define the current controlset

		// instantiate shared memory
		_internal::memory mem;

		// create a mutex to prevent critical section overlap
		std::mutex mutx;

		// Create gamespace with ruleset
		Gamespace thisGame(rules);

		// Start the game threads
		try {
			auto // Start asynchronous threads
				display{ std::async(std::launch::async, &_internal::thread_display, std::ref(mutx), std::ref(mem), std::ref(thisGame), std::ref(rules)) },
				enemy{ std::async(std::launch::async, &_internal::thread_npc, std::ref(mutx), std::ref(mem), std::ref(thisGame)) },
				player{ std::async(std::launch::async, &_internal::thread_player, std::ref(mutx), std::ref(mem), std::ref(thisGame)) };
		} catch ( std::exception & ex ) {
			sys::cls();
			msg(sys::error, "\"" + std::string(ex.what()) + "\" was thrown while starting the game threads.", "Press any key to exit....");
		}
		// Check if the restart prompt should be shown, and return
		if ( mem.kill_code.load() == _internal::PLAYER_QUIT_CODE )
			return false;
		return true;
	}
}