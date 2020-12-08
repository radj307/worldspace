#pragma once
#include "Coord.h"
#include "WinAPI.h"

// Base flare class
struct Flare {
protected:
	unsigned short
		_time,			// How many frames to display the flare for
		_color;			// which color to use for flare

	/**
	 * Flare(Coord&, unsigned short, unsigned short)
	 * @brief Construct a flare instance
	 *
	 * @param flareTime	 - How many frames total to flare the display. Should be a multiple of 2.
	 * @param flareColor - Which color to use, this should be a windows API color.
	 */
	Flare(const unsigned short flareTime, WinAPI::color flareColor) : _time(flareTime), _color(static_cast<unsigned short>(flareColor)), _max_time(_time) {}
	Flare(const unsigned short flareTime, const unsigned short flareColor) : _time(flareTime), _color(flareColor), _max_time(_time) {}

private:
	// This variable is a copy of the unmodified time value, do not change this.
	unsigned short _max_time;
public:
	/**
	 * pattern(int, int)
	 * @brief Pure virtual method that returns true if a tile should change color, false if it should not.
	 *
	 * @param x		 - Tile's X-axis index (horizontal)
	 * @param y		 - Tile's Y-axis index (vertical)
	 * @returns bool - ( true = flare this tile ) ( false = do not flare this tile )
	 */
	virtual bool pattern(int x, int y) = 0;

	/**
	 * time()
	 * @brief Returns the remaining flare time.
	 *
	 * @returns unsigned short
	 */
	[[nodiscard]] unsigned short time() const { return _time; }

	/**
	 * color()
	 * @brief Returns the flare color
	 *
	 * @returns unsigned short
	 */
	[[nodiscard]] unsigned short color() const { return _color; }

	// Decrement the remaining flare time by 1
	void decrement() { --_time; }

	// Resets the remaining time to max
	void reset() { _time = _max_time; }

	// default logistics functions
	Flare(const Flare&) = default;
	Flare(Flare&&) = default;
	virtual ~Flare() = default;
	Flare& operator=(const Flare&) = default;
	Flare& operator=(Flare&&) = default;
};

struct FlareClear final : Flare {
	bool pattern(const int x, const int y) override
	{
		return true;
	}

	FlareClear() : Flare(1, WinAPI::color::reset) {}
};

// Level-up flare
struct FlareLevel final : Flare {
	/**
	 * pattern(int, int)
	 * @brief Diagonal flare pattern for level-ups
	 *
	 * @param x		 - X-axis (horizontal)
	 * @param y		 - Y-axis (vertical)
	 * @returns bool - ( true = flare this tile ) ( false = do not flare this tile )
	 */
	bool pattern(const int x, const int y) override
	{
		return (x - y % 2) % 2 == 0;
	}

	/**
	 * FlareLevel()
	 * @brief Default constructor with preset color of BACKGROUND_GREEN, and a time of 6 frames.
	 */
	FlareLevel() : Flare(6, BACKGROUND_GREEN) {}

	/**
	 * FlareLevel(unsigned short, WinAPI::color)
	 * @brief Constructor with defined time & color values.
	 *
	 * @param flareTime  - The amount of frames to show a flare.
	 * @param flareColor - The color to flare.
	 */
	FlareLevel(const unsigned short flareTime, const WinAPI::color flareColor) : Flare(flareTime, flareColor) {}

	/**
	 * FlareLevel(unsigned short, WinAPI::color)
	 * @brief Constructor with defined time & color values.
	 *
	 * @param flareTime  - The amount of frames to show a flare.
	 * @param flareColor - The color to flare.
	 */
	FlareLevel(const unsigned short flareTime, const unsigned short flareColor) : Flare(flareTime, flareColor) {}
};

// Finale flare
struct FlareChallenge final : Flare {
private:
	Coord _cell_size;	// This is needed to determine the bottom & right edge indexes

public:
	/**
	 * pattern(int, int)
	 * Edge flare pattern for final challenge.
	 *
	 * @param x		 - X-axis (horizontal)
	 * @param y		 - Y-axis (vertical)
	 * @returns bool - ( true = flare this tile ) ( false = do not flare this tile )
	 */
	bool pattern(const int x, const int y) override
	{
		return x <= 0 || x >= _cell_size._x || y <= 0 || y >= _cell_size._y;
	}

	/**
	 * FlareChallenge(Coord&, unsigned short, WinAPI::color)
	 * @brief Constructor with defined time & color values.
	 *
	 * @param cellSize	 - A ref to the cell's _max member, representing the size of the tile matrix.
	 * @param flareTime  - The amount of frames to show a flare.
	 * @param flareColor - The color to flare.
	 */
	FlareChallenge(const Coord& cellSize, const unsigned short flareTime, const WinAPI::color flareColor) : Flare(flareTime, flareColor), _cell_size(cellSize._x - 1, cellSize._y - 1) {}

	/**
	 * FlareChallenge(Coord&, unsigned short, unsigned short)
	 * @brief Constructor with defined time & color values. This constructor takes a direct unsigned short.
	 *
	 * @param cellSize	 - A ref to the cell's _max member, representing the size of the tile matrix.
	 * @param flareTime  - The amount of frames to show a flare.
	 * @param flareColor - The color to flare.
	 */
	FlareChallenge(const Coord& cellSize, const unsigned short flareTime, const unsigned short flareColor) : Flare(flareTime, flareColor), _cell_size(cellSize._x - 1, cellSize._y - 1) {}

	/**
	 * FlareChallenge(Coord&)
	 * @brief Default constructor. Uses BACKGROUND_RED & time of 10 frames.
	 *
	 * @param cellSize	- A ref to the cell's _max member, representing the size of the tile matrix.
	 */
	explicit FlareChallenge(const Coord& cellSize) : Flare(10, BACKGROUND_RED), _cell_size(cellSize._x - 1, cellSize._y - 1) {}
};