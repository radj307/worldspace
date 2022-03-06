/**
 * Project "worldspace"
 * Wrapper for a matrix of tiles that can be used for simple games. Contains built-in console output functions, but can be used as a backend for graphical applications.
 * ~ Evolved into a basic real-time game application utilizing multi-threading.
 * @author radj307
 *
 *	[Task List]
		Add a "Killed by <name>" line to the game over screen
		Add a check when an NPC is pursuing its target to re-apply aggression if the target is still visible.
		Use the get() function to output localized flares rather than full-screen ones.
 */
#include "game.hpp"
#include <TermAPI.hpp>
#include <ParamsAPI2.hpp>

inline bool prompt_restart(const std::optional<const Coord>& textPos = std::nullopt);
/**
 * main(const int, char*[])
 * @brief Main program entry point.
 * @param argc	- Argument count
 * @param argv	- Argument array
 */
int main(const int argc, char* argv[])
{
	try {
		opt::ParamsAPI2 args{ argc, argv, "ini" };

		std::cout << term::EnableANSI << term::EnableAltScreenBuffer;
		// Keep starting the game until the player doesn't press restart
		do if (!game::start(args.typegetv_all<opt::Parameter>())) break; while (prompt_restart());

		// Return a success code
		return 0;
	} catch (std::exception& ex) {
		// Fill the screen buffer with spaces
		std::cout << term::DisableAltScreenBuffer << term::error << "The game crashed because an exception was thrown: \"" << ex.what() << "\"\nPress any key to exit." << std::endl;
		// Return an error code
		return -1;
	}
}

/**
 * prompt_restart(Coord)
 * @brief Prompts the user to press a key and returns true if the key was 'r', false if the key was 'q'.
 * @param textPos	- A location in the screen buffer where prompt message will be displayed.
 * @return true		- Player pressed the restart key.
 * @return false	- Player didn't press a key in time, or pressed the quit key.
 * @returns bool	- ( true = Restart the game ) ( false = Quit the game )
 */
bool prompt_restart(const std::optional<const Coord>& textPos)
{
	using namespace std::chrono_literals;
	using CLK = std::chrono::steady_clock; ///< @brief Chrono steady clock
	const auto timeout{ std::chrono::duration_cast<std::chrono::nanoseconds>(6s) };	///< @brief Maximum time to wait before returning false
	const auto pos{ textPos.has_value() ? textPos.value() : Coord{ static_cast<long>(40), 14L } };

	std::cout << term::setCursorPosition(pos._x, pos._y) << "Press <" << color::setcolor::green << 'r' << color::setcolor::reset << "> to restart.";

	std::cout << term::setCursorPosition(pos._x, pos._y + 1)<< "Press <" << color::setcolor::red << 'q' << color::setcolor::reset << "> to quit.";

	// loop until timeout, or valid key press
	for (const auto t{ CLK::now() }; CLK::now() - t <= timeout; ) {
		if (static_cast<bool>(term::kbhit())) { // If a key is pressed, process it
			switch (std::tolower(term::getch())) {
			case 'r': // r was pressed, restart the game
				return true;
			case 'q': // q was pressed, quit the game
				return false;
			default:break;
			}
		} // else wait for timeout
		term::setCursorPosition(pos._x + 2, pos._y + 3);
		std::cout << std::chrono::duration_cast<std::chrono::seconds>(timeout - (CLK::now() - t)).count() << "s remaining..." << std::endl;
	}
	return false;
}
