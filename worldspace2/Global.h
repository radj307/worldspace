#pragma once
#include <make_exception.hpp>
#include <str.hpp>
#include <indentor.hpp>

#include <chrono>
#include <filesystem>
#include <string>

struct undefined_exception : ex::except {
	undefined_exception(const std::string& threadName) : ex::except(str::stringify("An exception occurred in thread \"", threadName, "\"!\n", indent(10), "Exception:  [undefined]")) {}
};
struct thread_exception : ex::except {
	thread_exception(const std::string& threadName, const std::exception& innerException) : ex::except(str::stringify("An exception occurred in thread \"", threadName, "\"!\n", indent(10), "Exception:  ", innerException.what(), '\n')) {}
};

enum class GameState : unsigned char {
	NULL_STATE,		// null state
	INITIALIZING,	// game is starting up.
	RUNNING,		// game is running.
	PAUSED,			// game is paused.
	OVER,			// [EXIT]: Game is over.
	STOPPING,		// [EXIT]: Player stopped the game
	EXCEPTION,		// [EXIT]: Game crashed because of an exception, and should not be restarted.
};

/**
 * @brief		Checks if the given GameState is valid, and indicates that the game threads should continue running.
 * @param state	Input GameState
 * @returns		bool
 */
inline bool valid_state(const GameState& state)
{
	switch (state) {
	case GameState::INITIALIZING:
	case GameState::RUNNING:
	case GameState::PAUSED:
		return true;
	default:
		return false;
	}
}

static struct {
	std::filesystem::path myPath;
	std::string myName;

	const long long DEFAULT_SIZE_X{ 30 }, DEFAULT_SIZE_Y{ 30 };

	GameState state{ GameState::NULL_STATE };
	std::optional<std::exception> exception;

	std::chrono::milliseconds frametime{ 13 };
	std::chrono::milliseconds gametime{ 200 };
} Global;

/**
 * @brief			Sets the global frametime to the frametime required to meet the given target FPS.
 * @param newFPS	Target number of frames per second.
 */
inline static void set_framerate(const size_t& newFPS)
{
	Global.frametime = std::chrono::milliseconds(1000) / newFPS;
}

