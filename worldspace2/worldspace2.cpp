#include "Global.h"
#include "COLOR.h"
#include "resources/version.h"
#include "Controls.h"
#include "display/framebuffer.hpp"
#include "display/PauseMenu.hpp"
#include "display/GameOverMenu.hpp"
#include "actors/Actors.h"
#include "world/gamespace.hpp"
#include "world/framebuilder_matrix.hpp"
#include "world/framebuilder_gamespace.hpp"
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

using CLK = std::chrono::high_resolution_clock;
using DUR = std::chrono::duration<long double>;
using TIMEP = std::chrono::time_point<CLK, DUR>;

inline void thread_input(std::mutex& mtx, Controls& controls, gamespace& game) noexcept
{
	try {
		static const point UP{ 0, -1 }, DOWN{ 0, 1 }, LEFT{ -1, 0 }, RIGHT{ 1, 0 };
		for (auto& state{ Global.state }; valid_state(state); ) {
			if (term::kbhit()) {
				int key{ term::getch() };
				switch (controls.fromKey(key)) {
				case Control::SEQUENCE: {
					if (term::kbhit()) { //< this checks if there is still data in STDIN, not if a key is actually held down
						key = term::getch();
						std::scoped_lock<std::mutex> lock(mtx); // mutex lock is required when firing projectiles to prevent unexpected projectile additions/removals during display cycles
						switch (controls.fromKey(key)) {
						case Control::FIRE_UP:
							game.playerFireProjectile(UP);
							break;
						case Control::FIRE_DOWN:
							game.playerFireProjectile(DOWN);
							break;
						case Control::FIRE_LEFT:
							game.playerFireProjectile(LEFT);
							break;
						case Control::FIRE_RIGHT:
							game.playerFireProjectile(RIGHT);
							break;
						default:break;
						}
					}
					break;
				}
				case Control::PAUSE:
					if (state != GameState::PAUSED)
						state = GameState::PAUSED;
					else
						state = GameState::RUNNING;
					break;
				case Control::UP:
					game.movePlayer(UP);
					break;
				case Control::DOWN:
					game.movePlayer(DOWN);
					break;
				case Control::LEFT:
					game.movePlayer(LEFT);
					break;
				case Control::RIGHT:
					game.movePlayer(RIGHT);
					break;
				case Control::QUIT:
					state = GameState::STOPPING;
					break;
				default:break;
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
		const std::string& frametime_str{ "Frametime: " };
		point frametime_pos{ GameConfig.gridSize.x / 2 - (12), 0 };
		std::cout << term::setCursorPosition(frametime_pos) << frametime_str;
		frametime_pos.x += frametime_str.size();
		std::unique_ptr<PauseMenu> pauseMenu{ nullptr };
		for (auto& state{ Global.state }; valid_state(state); ) {
			const auto& t_start{ CLK::now() };
			switch (state) {
			case GameState::INITIALIZING:
			{ // critical section
				std::scoped_lock<std::mutex> lock(mtx);
				framebuf.initDisplay();
				if (pauseMenu == nullptr) {
					point pos{ static_cast<point>(term::getScreenBufferSize()) / 2 };
					pos.y /= 1.5;
					pauseMenu = std::make_unique<PauseMenu>(pos, color::setcolor::cyan);
				}
				Global.state = GameState::RUNNING;
				break;
			} // critical section
			case GameState::PAUSED:
			{ // critical section
				std::scoped_lock<std::mutex> lock(mtx);
				if (framebuf.isInitialized())
					framebuf.deinitDisplay(); // re-initialization is handled internally by the framebuffer
				if (pauseMenu.get() != nullptr)
					pauseMenu->display();
				break;
			} // critical section
			case GameState::RUNNING:
			{ // critical section
				std::scoped_lock<std::mutex> lock(mtx);
				framebuf.display();
				break;
			} // critical section
			default:break;
			}
			const auto& t_end{ CLK::now() };
			std::chrono::duration<double, std::milli> elapsed{ t_end - t_start };
			std::cout
				<< term::setCursorPosition(frametime_pos)
				<< elapsed.count()
				<< " ms      "
				;
			std::this_thread::sleep_until(t_end);
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
	using CLK = std::chrono::high_resolution_clock;
	try {
		for (auto& state{ Global.state }; valid_state(state); ) {
			const auto& t_start{ CLK::now() };
			switch (state) {
			case GameState::RUNNING:
			{ // critical section
				std::scoped_lock<std::mutex> lock(mtx);

				if (game.player.isDead())
					Global.state = GameState::OVER;

				game.PerformPeriodicRegen();
				break;
			} // critical section
			default:break;
			}
			std::this_thread::sleep_until(t_start + Global.regentime);
		}
	} catch (const std::exception& ex) {
		Global.state = GameState::EXCEPTION;
		Global.exception = thread_exception("Main Game Thread", ex);
		return;
	} catch (...) {
		Global.state = GameState::EXCEPTION;
		Global.exception = undefined_exception("Main Game Thread");
		return;
	}
}
inline void thread_npc(std::mutex& mtx, gamespace& game) noexcept
{
	try {
		using CLK = std::chrono::high_resolution_clock;
		for (auto& state{ Global.state }; valid_state(state);) {
			const auto tBeginCycle{ CLK::now() };

			switch (state) {
			case GameState::RUNNING:
			{ // critical section
				std::scoped_lock<std::mutex> lock(mtx);
				game.ProcessProjectileActions();
				game.PerformActionAllNPCs();
				break;
			} // critical section
			default:break;
			}

			std::this_thread::sleep_until(tBeginCycle + Global.gametime);
		}
	} catch (const std::exception& ex) {
		Global.state = GameState::EXCEPTION;
		Global.exception = thread_exception("AI Processing Thread", ex);
		return;
	} catch (...) {
		Global.state = GameState::EXCEPTION;
		Global.exception = undefined_exception("AI Processing Thread");
		return;
	}
}

/**
 * @brief
 * @param controls
 * @param timeout
 * @returns				bool
 *						true:	Restart key pressed.
 *						false:	Quit key pressed, or timeout reached.
 */
inline bool handleGameOver(Controls& controls, const std::chrono::milliseconds& timeout)
{
	if (Global.state == GameState::EXCEPTION) {
		if (Global.exception.has_value())
			throw Global.exception.value();
		else throw make_exception("An unknown exception occurred!");
	}

	const auto& tStart{ std::chrono::high_resolution_clock::now() };

	point pos{ static_cast<point>(term::getScreenBufferSize()) / 2 };
	pos.y /= 1.5;

	GameOverMenu menu{ pos, controls };
	menu.display();

	pos.y += menu.height() + 1ull;

	const auto& timeColorSegment{ timeout / 3 };
	const auto& selectColor{ [&timeColorSegment](const std::chrono::nanoseconds& elapsed) {
		if (elapsed > timeColorSegment * 2)
			return color::setcolor::red;
		else if (elapsed > timeColorSegment)
			return color::setcolor{ color::orange };
		else
			return color::setcolor::green;
	} };

	const std::string s_app{ " ms remaining..." };
	for (auto elapsed{ std::chrono::high_resolution_clock::now() - tStart }; elapsed <= timeout; elapsed = std::chrono::high_resolution_clock::now() - tStart) {
		if (term::kbhit()) {
			int key{ term::getch() };
			switch (controls.fromKey(key)) {
			case Control::RESTART:
				std::cout << term::clear;
				return true;
			case Control::QUIT:
				return false;
			default:
				break;
			}
		}
		const auto& s{ str::stringify(std::chrono::duration_cast<std::chrono::milliseconds>(timeout - elapsed).count()) };
		std::cout << term::setCursorPosition(point{ pos.x - (static_cast<int>(s.size()) / 2) - (static_cast<int>(s_app.size() / 2)), pos.y }) << ' ' << selectColor(elapsed) << s << color::reset << s_app << ' ';
	}
	return false;
}

#ifdef OS_WIN
#include <Windows.h>
LONG SEH_Handler(_EXCEPTION_POINTERS* exInfo)
{
	const auto& ex{ exInfo->ExceptionRecord };

	Global.state = GameState::EXCEPTION;
	Global.exception = make_exception("Unhandled SEH exception '", ex->ExceptionCode, "'\n");

	return EXCEPTION_CONTINUE_EXECUTION;
}
#endif

int main(const int argc, char** argv)
{
#ifdef OS_WIN // Handle 'Structured Exception Handle' exceptions on windows (these aren't caught by try-catch blocks, and could cause threads to fail without notice)
	AddVectoredExceptionHandler(1, SEH_Handler);
#endif
	try {
		// enable ANSI escape sequences
		std::cout << term::EnableANSI;

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

		GameConfig.player_template.setName("radj");

		// Swap to alternate screen buffer
		std::cout << term::EnableAltScreenBuffer << term::CursorVisible(false);


		// initialize controls & ini config
		Controls controls;
		INI ini{ Global.myPath };

		point gridSize{ ini.getvs_cast<position>("game", "iGridSizeX", str::stoi).value_or(Global.DEFAULT_SIZE_X), ini.getvs_cast<position>("game", "iGridSizeY", str::stoi).value_or(Global.DEFAULT_SIZE_Y) };

		std::mutex mutex;

		const auto& timeStart{ std::chrono::high_resolution_clock::now() };

		do {
			Global.state = GameState::INITIALIZING;

			gamespace game{};

			framebuffer framebuf{ gridSize };
			//framebuf.setBuilder<framebuilder_matrix>(game.grid);
			framebuf.setBuilder<framebuilder_gamespace>(game, framebuilder_gamespace_config{ false, true });
			framebuf.setLinker<framelinker_gamespace>(game);
			framebuf.setPanel<statpanel>(&game.player);
			framebuf.initDisplay();

			auto
				t_input{ std::async(std::launch::async, thread_input, std::ref(mutex), std::ref(controls), std::ref(game)) },
				t_display{ std::async(std::launch::async, thread_display, std::ref(mutex), std::ref(framebuf)) },
				t_game{ std::async(std::launch::async, thread_game, std::ref(mutex), std::ref(game)) },
				t_npc{ std::async(std::launch::async, thread_npc, std::ref(mutex), std::ref(game)) };

			t_input.wait();
			t_display.wait();
			t_game.wait();
		} while (handleGameOver(controls, Global.restartTimeout));

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
