#include "Global.h"
#include "COLOR.h"
#include "resources/version.h"
#include "Controls.h"
#include "display/frame.hpp"
#include "display/debug.h"

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

inline void thread_input(std::mutex& mtx, Controls& controls) noexcept
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
					std::cout << static_cast<char>(key) << ' ';
					break;
				case Control::DOWN:
					std::cout << static_cast<char>(key) << ' ';
					break;
				case Control::LEFT:
					std::cout << static_cast<char>(key) << ' ';
					break;
				case Control::RIGHT:
					std::cout << static_cast<char>(key) << ' ';
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
	} catch (...) {
		Global.state = GameState::EXCEPTION;
		Global.exception = undefined_exception("Input");
	}
}
inline void thread_display(std::mutex& mtx, framebuffer& framebuf) noexcept
{
	try {
		for (auto& state{ Global.state }; valid_state(state); ) {
			switch (state) {
			case GameState::INITIALIZING:
				framebuf.initDisplay();
				Global.state = GameState::RUNNING;
				break;
			case GameState::PAUSED:
				// TODO: Display the pause menu
				break;
			case GameState::RUNNING:
				framebuf.display();
				break;
			default:break;
			}
			std::this_thread::sleep_for(Global.frametime);
		}
	} catch (const std::exception& ex) {
		Global.state = GameState::EXCEPTION;
		Global.exception = thread_exception("Display", ex);
	} catch (...) {
		Global.state = GameState::EXCEPTION;
		Global.exception = undefined_exception("Display");
	}
}
inline void thread_ai(std::mutex& mtx) noexcept
{
	try {
		for (auto& state{ Global.state }; valid_state(state); ) {
			// TODO: Implement AI processing
		}
	} catch (const std::exception& ex) {
		Global.state = GameState::EXCEPTION;
		Global.exception = thread_exception("AI Control", ex);
	} catch (...) {
		Global.state = GameState::EXCEPTION;
		Global.exception = undefined_exception("AI Control");
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
		point gridSize{ ini.getvs_cast<position>("game", "iGridSizeX", str::stoll).value_or(Global.DEFAULT_SIZE_X), ini.getvs_cast<position>("game", "iGridSizeY", str::stoll).value_or(Global.DEFAULT_SIZE_Y) };

		framebuffer framebuf{ gridSize };
		framebuf.setBuilder<framebuilder_debug>();
		framebuf.setLinker<framelinker_debug>();

		std::mutex mutex;

		const auto& timeStart{ std::chrono::high_resolution_clock::now() };

		auto
			t_input{ std::async(std::launch::async, thread_input, std::ref(mutex), std::ref(controls)) },
			t_display{ std::async(std::launch::async, thread_display, std::ref(mutex), std::ref(framebuf)) },
			t_ai{ std::async(std::launch::async, thread_ai, std::ref(mutex)) };

		t_input.wait();
		t_display.wait();
		t_ai.wait();

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
