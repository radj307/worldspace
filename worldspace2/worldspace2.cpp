#include "Global.h"
#include "COLOR.h"
#include "resources/version.h"
#include "Controls.h"
#include "display/framebuffer.hpp"
#include "display/PauseMenu.hpp"
#include "actors/Actors.h"
#include "world/gamespace.hpp"
#include "world/framebuilder_matrix.hpp"
#include "world/framelinker_gamespace.hpp"

#include <INI.hpp>
#include <envpath.hpp>
#include <ParamsAPI2.hpp>
#include <TermAPI.hpp>
#include <palette.hpp>

using file::INI;

#include <mutex>
#include <future>

inline std::ostream& help(std::ostream& os)
{
	return os
		<< Global.myName << (Global.myName != "worldspace2" ? "(worldspace2)" : "") << "  v" << worldspace2_VERSION << '\n'
		<< "  Terminal-based live action roguelike game." << '\n'
		<< '\n'
		<< "USAGE:" << '\n'
		<< "  worldspace2 [OPTIONS]" << '\n'
		<< '\n'
		<< "OPTIONS:" << '\n'
		<< "  -h  --help                  Print this help display, then exit." << '\n'
		<< "  -v  --version               Print the current version number, then exit." << '\n'
		<< "  " << '\n'
		;
}

inline void thread_input(std::mutex& mtx, Controls& controls, gamespace& game) noexcept
{
	try {
		for (auto& state{ Global.state }; valid_state(state); ) {
			if (term::kbhit()) {
				int key{ term::getch() };
				switch (controls.fromKey(key)) {
				case Control::PAUSE:
					if (state != GameState::PAUSED)
						state = GameState::PAUSED;
					else
						state = GameState::RUNNING;
					break;
				case Control::UP:
					game.movePlayer(point{ 0, -1 });
					break;
				case Control::DOWN:
					game.movePlayer(point{ 0, 1 });
					break;
				case Control::LEFT:
					game.movePlayer(point{ -1, 0 });
					break;
				case Control::RIGHT:
					game.movePlayer(point{ 1, 0 });
					break;
				case Control::QUIT:
					state = GameState::STOPPING;
					break;
				default:
					break;
				}
			}
		}
	} catch (const std::exception& ex) {
		Global.state = GameState::EXCEPTION;
		Global.exception = thread_exception("Input", ex);
		return;
	} catch (...) {
		Global.state = GameState::EXCEPTION;
		Global.exception = undefined_exception("Input");
		return;
	}
}
inline void thread_display(std::mutex& mtx, framebuffer& framebuf) noexcept
{
	try {
		std::unique_ptr<PauseMenu> pauseMenu{ nullptr };
		for (auto& state{ Global.state }; valid_state(state); ) {
			const auto& t_start{ std::chrono::high_resolution_clock::now() };
			std::scoped_lock<std::mutex> lock(mtx);
			switch (state) {
			case GameState::INITIALIZING:
				framebuf.initDisplay();
				if (pauseMenu == nullptr) {
					point pos{ static_cast<point>(term::getScreenBufferSize()) / 2 };
					pos.y /= 1.5;
					pauseMenu = std::make_unique<PauseMenu>(pos, color::setcolor::cyan);
				}
				Global.state = GameState::RUNNING;
				break;
			case GameState::PAUSED:
				if (framebuf.isInitialized())
					framebuf.deinitDisplay(); // re-initialization is handled internally by the framebuffer
				if (pauseMenu.get() != nullptr)
					pauseMenu->display();
				break;
			case GameState::RUNNING:
				framebuf.display();
				break;
			default:break;
			}
			std::this_thread::sleep_until(t_start + Global.frametime);
		}
	} catch (const std::exception& ex) {
		Global.state = GameState::EXCEPTION;
		Global.exception = thread_exception("Display", ex);
		return;
	} catch (...) {
		Global.state = GameState::EXCEPTION;
		Global.exception = undefined_exception("Display");
		return;
	}
}
inline void thread_game(std::mutex& mtx, gamespace& game) noexcept
{
	try {
		for (auto& state{ Global.state }; valid_state(state); ) {
			const auto& t_start{ std::chrono::high_resolution_clock::now() };
			switch (state) {
			case GameState::RUNNING: {
				std::scoped_lock<std::mutex> lock(mtx);
				game.PerformActionAllNPCs();

				if (game.player.isDead())
					Global.state = GameState::OVER;
				break;
			}
			default:break;
			}
			std::this_thread::sleep_until(t_start + Global.gametime);
			// TODO: Implement AI processing
		}
	} catch (const std::exception& ex) {
		Global.state = GameState::EXCEPTION;
		Global.exception = thread_exception("AI Control", ex);
		return;
	} catch (...) {
		Global.state = GameState::EXCEPTION;
		Global.exception = undefined_exception("AI Control");
		return;
	}
}

int main(const int argc, char** argv)
{
	try {
		// enable ANSI escape sequences
		std::cout << term::EnableANSI;

		const auto& scbSize{ term::getScreenBufferSize() };

		// parse arguments, resolve path
		opt::ParamsAPI2 args{ argc, argv };
		env::PATH path{ argv[0] };
		[&path](const char* const argv0) { // set global program path & name from env
			auto [myPath, myName] {path.resolve_split(argv0)};
			Global.myPath = myPath;
			Global.myName = myName.replace_extension().generic_string();
		}(argv[0]);

		// [ -h | --help ]
		if (args.check_any<opt::Flag, opt::Option>('h', "help")) {
			std::cout << help << std::endl;
			return EXIT_SUCCESS;
		}
		// [ -v | --version ]
		else if (args.check_any<opt::Flag, opt::Option>('v', "version")) {
			std::cout << worldspace2_VERSION << std::endl;
			return EXIT_SUCCESS;
		}

		// Swap to alternate screen buffer
		std::cout << term::EnableAltScreenBuffer << term::CursorVisible(false);

		// set the global state to initializing
		Global.state = GameState::INITIALIZING;

		// initialize controls & ini config
		Controls controls;
		INI ini{ Global.myPath };

		point gridSize{ ini.getvs_cast<position>("game", "iGridSizeX", str::stoi).value_or(Global.DEFAULT_SIZE_X), ini.getvs_cast<position>("game", "iGridSizeY", str::stoi).value_or(Global.DEFAULT_SIZE_Y) };

		std::mutex mutex;

		const auto& timeStart{ std::chrono::high_resolution_clock::now() };

		gamespace game;

		/// DEBUGGING ///
		game.pathFind({ 0, 0 }, { 3, 3 });
		/// DEBUGGING ///

		framebuffer framebuf{ gridSize };
		framebuf.setBuilder<framebuilder_matrix>(game.grid);
		framebuf.setLinker<framelinker_gamespace>(game);
		framebuf.setPanel<statpanel>(&game.player);

		auto
			t_input{ std::async(std::launch::async, thread_input, std::ref(mutex), std::ref(controls), std::ref(game)) },
			t_display{ std::async(std::launch::async, thread_display, std::ref(mutex), std::ref(framebuf)) },
			t_game{ std::async(std::launch::async, thread_game, std::ref(mutex), std::ref(game)) };

		t_input.wait();
		t_display.wait();
		t_game.wait();

		const auto& timeEnd{ std::chrono::high_resolution_clock::now() };

		// swap to main screen buffer
		std::cout << term::DisableAltScreenBuffer;

		if (Global.exception.has_value())
			throw Global.exception;
		else std::cout
			<< "Successfully exited after "
			<< std::chrono::duration_cast<std::chrono::duration<long double, std::ratio<60L>>>(
				std::chrono::duration<long double>(timeEnd - timeStart)
			).count() << " minutes." << std::endl;

		return EXIT_SUCCESS;
	} catch (const std::exception& ex) {
		std::cerr << palette.get_error() << ex.what() << std::endl;
		return EXIT_FAILURE;
	} catch (...) {
		std::cerr << palette.get_error() << "An undefined exception occurred!" << std::endl;
		return EXIT_FAILURE;
	}
}
