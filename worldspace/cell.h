/**
 * @file cell.h
 * @brief Represents the game world.
 * Contains the Cell class, and the Tile struct.
 * @author radj307
 */
// ReSharper disable CppClangTidyClangDiagnosticDocumentationUnknownCommand
#pragma once
#include <vector>
#include <xRand.hpp>

#include "Coord.h"

/**
 * @struct Tile
 * @brief Represents a single position in the matrix of a cell. \n
 */
struct Tile final {
private:
	/**
	 * initTraits(bool)
	 * @brief Initialize this tile's variables based on type.
	 */
	void initTraits() noexcept
	{
		// check if tile attributes are correct
		switch ( _display ) {  // NOLINT(clang-diagnostic-switch-enum)
		case display::hole: _canMove = true;
			_isTrap = true;
			break;
		case display::empty: _canMove = true;
			_canSpawn = true;
			break;
		default: break;
		}
	}

public:

	/**
	 * @enum display
	 * @brief Defines valid tile types/display characters.
	 */
	enum class display {
		empty = '_',
		wall = '#',
		hole = 'O',
		none = '?',
	};

	display _display;	///< @brief Determines this tile's appearance when displayed to the console.
	bool _isKnown;		///< @brief Determines if this tile is visible to the player. When true, this tile is visible.
	bool _canMove;		///< @brief Determines if actors can move to this tile from an adjacent tile. When true, actors can move here.
	bool _isTrap;		///< @brief Determines if this tile is a trap or not. When true, this tile is considered a trap.
	bool _canSpawn;		///< @brief Determines if entities are allowed to spawn on this tile. When true, entities can spawn here.

	/** CONSTRUCTOR **
	 * Tile(Tile::display, int, int, bool)
	 * @brief Construct a tile with the default color.
	 * @param as		 - This tile's type (display character)
	 * @param isVisible	 - When true, this tile is visible to the player by default
	 */
	Tile( const display as, const bool isVisible ) noexcept : _display( as ), _isKnown( isVisible ), _canMove( false ), _isTrap( false ), _canSpawn( false ) { initTraits(); }
};

/**
 * @class Cell
 * @brief Represents the environment of the gamespace. \n
 * The cell contains the tile matrix, it does not have any knowledge of the gamespace or entities located within it. \n
 * The cell does not have the ability to display itself to the console.
 */
class Cell final {
	using row = std::vector<Tile>;	///< @brief A single vector of Tile instances.
	using cell = std::vector<row>;	///< @brief A vector of row instances, which creates a 2-D matrix.

	cell _matrix;	///< @brief Tile matrix
	bool // These booleans determine the visibility of specific tile types when the game starts.
		_vis_all,	///< @brief Determines if the player can see all Tile instances when the game starts.
		_vis_wall;	///< @brief Determines if the player can see all Tile instances that are walls when the game starts.

	/**
	 * isAdjacent(Tile::display, Coord&)
	 * @brief Checks the tiles surrounding a given position for a target type.
	 * @param type	 - Tile type to check for
	 * @param pos	 - Target position
	 * @return true	 - At least one adjacent Tile is of the target type.
	 * @return false - No adjacent Tiles are of the target type.
	 */
	bool isAdjacent( const Tile::display type, const Coord& pos ) noexcept
	{
		for ( auto y{ pos._y - 1 }; y <= pos._y + 1; ++y )
			for ( auto x{ pos._x - 1 }; x <= pos._x + 1; ++x )
				if ( isValidPos( x, y ) && get( x, y )->_display == type && !( x == pos._x && y == pos._y ) )
					return true;
		return false;
	}

	/**
	 * generate()
	 * @brief Randomly generates the Tile matrix using the xRand.h lib.
	 */
	void generate()
	{
		_matrix.reserve( _max._y );
		if ( _max._y >= 10 && _max._x >= 10 ) {
			rng::tRand rng;
			for ( auto y = 0; y < _max._y; y++ ) {
				row row;
				row.reserve( _max._x );
				for ( auto x = 0; x < _max._x; x++ ) {
					// make walls on all edges
					if ( x == 0 || x == _max._x - 1 || ( y == 0 || y == _max._y - 1 ) )
						row.push_back( { Tile::display::wall, _vis_wall || _vis_all } );
					else { // not an edge
						const auto rand{ rng.get( 100.0f, 0.0f ) };
						if ( rand < 7.0f ) // 7:100 chance of a wall tile that isn't on an edge
							row.push_back( { Tile::display::wall, _vis_wall || _vis_all } );
						else if ( rand > 7.0f && rand < 9.0f )
							row.push_back( { Tile::display::hole, _vis_all } );
						else
							row.push_back( { Tile::display::empty, _vis_all } );
					}
				}
				_matrix.emplace_back( std::move( row ) );
			}
		}
	}

public:
	const Coord _max;	///< @brief This is the max point of the Cell, which is the bottom-right corner.
	// ReSharper disable once CppInconsistentNaming
	const checkBounds isValidPos; ///< @brief Functor that can be used to check if a point is within the boundaries of the Cell.

	/**
	 * Cell(Coord, bool)
	 * @brief Generate a new cell with the given size parameters. Minimum size is 10x10
	 * @param cellSize				- The size of the cell
	 * @param makeWallsVisible		- walls are always visible
	 * @param override_known_tiles	- When true, all tiles will be visible to the player from the start.
	 */
	explicit Cell( const Coord& cellSize, const bool makeWallsVisible = true, const bool override_known_tiles = false ) noexcept : _vis_all( override_known_tiles ), _vis_wall( makeWallsVisible ), _max( cellSize._x - 1, cellSize._y - 1 ), isValidPos( _max ) { try { generate(); } catch ( ... ) {} }

	/**
	 * getChar(Coord&)
	 * @brief Returns the display character of a given Tile.
	 * @param pos	 - Target position
	 * @return ' '	 - Invalid position
	 * @returns char - The display char of the Tile located at the given position.
	 */
	char getChar( const Coord& pos ) noexcept { return isValidPos( pos ) ? static_cast<char>(get( pos )->_display) : ' '; }

	/**
	 * modVis(bool)
	 * @brief Modifies the visibility of all tiles in the cell.
	 * @param to	- When true, Tiles are set to visible.
	 */
	void modVis( const bool to ) noexcept
	{
		if ( !_vis_all || to )
			for ( auto& y : _matrix )
				for ( auto& x : y ) {
					if ( x._display == Tile::display::wall )
						x._isKnown = to || _vis_wall;
					else
						x._isKnown = to;
				}
	}

	/**
	 * modVis(bool, long, long)
	 * @brief Modifies the visibility of a given Tile.
	 * @param to		- When true, Tile is set to visible.
	 * @param X			- X-axis (horizontal) index.
	 * @param Y			- Y-axis (vertical) index.
	 */
	void modVis( const bool to, const long X, const long Y ) noexcept
	{
		if ( isValidPos( X, Y ) ) {
			if ( _matrix.at( Y ).at( X )._display != Tile::display::wall )
				_matrix.at( Y ).at( X )._isKnown = to || _vis_all;
			else
				_matrix.at( Y ).at( X )._isKnown = to || _vis_wall;
		}
	}

	/**
	 * modVis(bool, Coord, int)
	 * @brief Modifies the visibility of a square area around a given center-point in the cell.
	 * @param to		- ( true = visible ) ( false = invisible )
	 * @param pos		- The center-point
	 * @param radius	- The distance away from the center-point that will also be discovered.
	 */
	void modVis( const bool to, const Coord& pos, const int radius ) noexcept
	{
		if ( !_vis_all || to )
			for ( int y = pos._y - radius; y <= pos._y + radius; y++ )
				for ( int x = pos._x - radius; x <= pos._x + radius; x++ )
					modVis( to, x, y );
	}

	/**
	 * modVis(bool, Coord, const int)
	 * @brief Modifies the visibility of a circular area around a given center-point in the cell.
	 * @param to		- ( true = visible ) ( false = invisible )
	 * @param pos		- The center-point
	 * @param radius	- The distance away from the center-point that will also be discovered.
	 */
	void modVisCircle( const bool to, const Coord& pos, const int radius ) noexcept
	{
		if ( !_vis_all || to )
			for ( int y = pos._y - radius; y <= pos._y + radius; y++ )
				for ( int x = pos._x - radius; x <= pos._x + radius; x++ )
					if ( checkDistance::get( x, y, pos, radius ) )
						modVis( to, x, y );
	}

	/**
	 * modVis(bool, Coord, Coord)
	 * @brief Modifies the visibility of a specified area in the cell.
	 * @param to		- ( true = visible ) ( false = invisible )
	 * @param minPos	- The top-left corner of the target area
	 * @param maxPos	- The bottom-right corner of the target area
	 */
	void modVis( const bool to, const Coord& minPos, const Coord& maxPos ) noexcept
	{
		if ( !_vis_all || to )
			for ( int y = minPos._y; y <= maxPos._y; y++ )
				for ( int x = minPos._x; x <= maxPos._x; x++ )
					modVis( to, x, y );
	}

	/**
	 * get(Coord, const bool)
	 * @brief Returns a pointer to the target tile.
	 * @param pos		- The target Tiles position.
	 * @returns Tile*
	 */
	Tile* get( const Coord& pos ) noexcept { return isValidPos( pos ) ? &_matrix.at( pos._y ).at( pos._x ) : nullptr; }

	/**
	 * get(Coord, const bool)
	 * @brief Returns a pointer to the target tile.
	 * @param x			- The target tile's x index
	 * @param y			- The target tile's y index
	 * @returns Tile*
	 */
	Tile* get( const int x, const int y ) noexcept { return isValidPos( x, y ) ? &_matrix.at( y ).at( x ) : nullptr; }
};
