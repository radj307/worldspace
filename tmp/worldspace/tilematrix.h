/**
 * @file tilematrix.h
 * @author radj307
 * @brief (UNIMPLEMENTED) Alternative implementation of cell.h which uses smart pointers & tile type inheritance.
 */
#pragma once
#include <vector>
#include <xRand.h>

#include "Coord.h"

struct TileTemplate final {
	char _char;
	bool _is_visible, _can_move, _is_trap, _can_spawn;

	TileTemplate( const char ch, const bool isVisible, const bool canMove, const bool isTrap, const bool canSpawn ) : _char{ ch }, _is_visible{ isVisible }, _can_move{ canMove }, _is_trap{ isTrap }, _can_spawn{ canSpawn }
	{
	}
};

enum class TILE_TYPE {
	EMPTY = '_',
	WALL = '#',
	TRAP = 'O',
	DOOR = '@',
	NONE = '?'
};

struct TileTraits {
protected:
	const TILE_TYPE _type;
	bool _is_visible;
	const bool _can_move, _is_trap, _can_spawn;

	TileTraits( const TILE_TYPE ch, const bool isVisible, const bool canMove, const bool isTrap, const bool canSpawn ) noexcept : _type{ ch }, _is_visible{ isVisible }, _can_move{ canMove }, _is_trap{ isTrap }, _can_spawn{ canSpawn }
	{
	}

	virtual ~TileTraits() noexcept = default;

public:
	TileTraits( const TileTraits& ) = delete;
	TileTraits( TileTraits&& ) = delete;
	TileTraits& operator=( const TileTraits& ) = delete;
	TileTraits& operator=( TileTraits&& ) = delete;

	[[nodiscard]] TILE_TYPE type() const { return _type; }
	[[nodiscard]] char getChar() const { return static_cast<auto>(_type); }
	[[nodiscard]] bool isVisible() const { return _is_visible; }
	[[nodiscard]] bool isTrap() const { return _is_trap; }
	[[nodiscard]] bool canMove() const { return _can_move; }
	[[nodiscard]] bool canSpawn() const { return _can_spawn; }

	bool modVis( const bool& visible ) { return _is_visible = visible; }
};

struct TileTrap final : TileTraits {
	int _dmg;

	explicit TileTrap( const int damage = 25 ) noexcept : TileTraits{ TILE_TYPE::TRAP, false, true, true, false }, _dmg( damage )
	{
	}
};

struct TileEmpty final : TileTraits {
	explicit TileEmpty() noexcept : TileTraits{ TILE_TYPE::EMPTY, false, true, false, true }
	{
	}
};

struct TileWall final : TileTraits {
	explicit TileWall() noexcept : TileTraits{ TILE_TYPE::WALL, false, false, false, false }
	{
	}
};

struct TileDoor final : TileTraits {
	explicit TileDoor() noexcept : TileTraits{ TILE_TYPE::DOOR, false, true, false, false }
	{
	}
};

struct TileMatrix {
private:
	using Row = std::vector<std::unique_ptr<TileTraits> >;
	using Col = std::vector<Row>;

	Col _matrix;
	const Coord _size;

	const bool _vis_all, _vis_wall;


	static Col generate( const Coord& size ) noexcept
	{
		Col matrix;
		try {
			const auto isEdge{ [&size]( const int x, const int y ) { return ( x == 0 || y == 0 || x == size._x - 1 || y == size._y - 1 ); } };
			tRand rng;
			matrix.reserve( size._y );
			for ( auto y{ 0 }; y < size._y; ++y ) {
				Row row;
				row.reserve( size._x );
				for ( auto x{ 0 }; x < size._x; ++x ) {
					if ( isEdge( x, y ) )
						row.emplace_back( std::make_unique<TileTraits>( TileWall{} ) );
					else {
						const auto random{ rng.get( 100.0f, 0.0f ) };
						if ( random <= 7.0f )
							row.emplace_back( std::make_unique<TileTraits>( TileWall{} ) );
						else if ( random <= 9.0f )
							row.emplace_back( std::make_unique<TileTraits>( TileTrap{} ) );
						else
							row.emplace_back( std::make_unique<TileTraits>( TileEmpty{} ) );
					}
				}
				row.shrink_to_fit();
				matrix.emplace_back( row );
			}
			matrix.shrink_to_fit();
			return matrix;
		} catch ( ... ) { return {}; }
	}

public:
	explicit TileMatrix( const Coord& cell_size, const bool all_visible = false, const bool wall_visible = true ) noexcept : _matrix{ generate( cell_size ) }, _size{ cell_size }, _vis_all{ all_visible }, _vis_wall{ wall_visible }
	{
	}

	// ReSharper disable once CppInconsistentNaming
	const auto isValidPos{ [this]( const Coord& pos ) -> bool { return pos._x >= 0 && pos._y >= 0 && pos._x <= _size._x - 1 && pos._y <= _size._y - 1; } };
	// ReSharper disable once CppInconsistentNaming
	const auto isValidPos{ [this]( const int x, const int y ) -> bool { return x >= 0 && y >= 0 && x <= _size._x - 1 && y <= _size._y - 1; } };

	void modVis( const bool visible ) noexcept
	{
		if ( !_vis_all || visible ) {
			for ( auto& y : _matrix ) {
				for ( auto& x : y ) {
					if ( x->type() == TILE_TYPE::WALL )
						x->modVis( visible || _vis_wall );
					else
						x->modVis( visible );
				}
			}
		}
	}

	void modVis( const bool visible, const int x, const int y ) noexcept
	{
		if ( isValidPos( x, y ) && ( !_vis_all || visible ) )
			_matrix.at( y ).at( x )->modVis( visible );
	}

	void modVis( const bool visible, const Coord& pos, const unsigned int radius ) noexcept
	{
		if ( isValidPos( pos ) && ( !_vis_all || visible ) )
			for ( auto y{ pos._y - radius }; y <= pos._y + radius; ++y )
				for ( auto x{ pos._x - radius }; x <= pos._x + radius; ++x )
					if ( checkDistance::get( x, y, pos, radius ) )
						modVis( visible, x, y );
	}

	void modVis( const bool visible, const Coord& origin, const Coord& max )
	{
		if ( isValidPos( origin ) && isValidPos( max ) && ( !_vis_all || visible ) ) {
			for ( auto y{ _matrix.begin() + origin._y }; y != _matrix.begin() + max._y; ++y ) {
				for ( auto x{ y->begin() + origin._x }; x != y->begin() + max._x; ++x ) {
					if ( x->get()->type() == TILE_TYPE::WALL )
						x->get()->modVis( visible || _vis_wall );
					else
						x->get()->modVis( visible );
				}
			}
		}
	}

	// GETTERS
	[[nodiscard]] char getChar( const Coord& pos ) noexcept { return isValidPos( pos ) ? static_cast<char>(_matrix.at( pos._y ).at( pos._x )->getChar()) : ' '; }
	[[nodiscard]] bool canSpawn( const int x, const int y ) const { return isValidPos( x, y ) && _matrix.at( y ).at( x )->canSpawn(); }
	[[nodiscard]] bool canSpawn( const Coord& pos ) const { return canSpawn( pos._x, pos._y ); }
	[[nodiscard]] bool canMove( const int x, const int y ) const { return isValidPos( x, y ) && _matrix.at( y ).at( x )->canMove(); }
	[[nodiscard]] bool canMove( const Coord& pos ) const { return canMove( pos._x, pos._y ); }
	[[nodiscard]] bool isTrap( const int x, const int y ) const { return isValidPos( x, y ) && _matrix.at( y ).at( x )->isTrap(); }
	[[nodiscard]] bool isTrap( const Coord& pos ) const { return isTrap( pos._x, pos._y ); }
	[[nodiscard]] bool isVisible( const int x, const int y ) const { return isValidPos( x, y ) && _matrix.at( y ).at( x )->isVisible(); }
	[[nodiscard]] bool isVisible( const Coord& pos ) const { return isVisible( pos._x, pos._y ); }
};
