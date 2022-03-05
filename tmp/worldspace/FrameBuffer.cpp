/**
 * @file FrameBuffer.cpp
 * @author radj307
 * @brief Contains the implementation of FrameBuffer.h
 */
#include "FrameBuffer.h"
#include <sysapi.h>
#include <utility>

/**
 * rebuildCache()
 * @brief Refreshes the cache from current gamespace data.
 */
void FrameBuffer::rebuildCache() noexcept
{
//	try {
		const auto size{ _cache.size() };
		_cache.clear();
		_cache.reserve( size );
		for ( auto& it : _game.get_all_actors() )
			_cache.emplace_back( std::make_tuple( it->pos(), it->getChar(), it->getColor() ) );
		for ( auto& it : _game.get_all_static_items() )
			_cache.emplace_back( std::make_tuple( it->pos(), it->getChar(), it->getColor() ) );
		_cache.shrink_to_fit();
//	} catch ( ... ) {
//	}
}


/**
 * initFrame(bool)
 * @brief Initializes the frame display. Throws std::exception if cell size is empty.
 * @throws std::exception("Cannot initialize an empty cell!") if the cell is empty.
 * @param doCLS	 - (Default: true) When true, the screen buffer is overwritten with blank spaces before initializing the frame.
 */
void FrameBuffer::initFrame( const bool doCLS )
{
	if ( !_initialized ) {
		if ( _game.getCellSize()._x > 0 && _game.getCellSize()._y > 0 ) {
			if ( doCLS )
				sys::cls();			// Clear the screen before initializing
			_last = buildNextFrame( _origin );	// set the last frame
			_last.draw();		// draw frame
			_initialized = true;	// set init frame boolean
		}
		else
			throw std::exception( "Cannot initialize an empty cell!" );
	} // else do nothing
}

/**
 * checkPos(Coord&)
 * @brief Checks a given position for matches in the cache.
 * @param pos							- Target position.
 * @return nullopt						- No entities are located at the given position.
 * @return pair<char, unsigned short>	- The display character & color of the entity located at the given position.
 */
std::optional<std::pair<char, unsigned short> > FrameBuffer::checkPos( const Coord& pos ) const noexcept
{
	for ( const auto& it : _cache )
		if ( pos == std::get<0>( it ) )
			return std::make_pair( std::get<1>( it ), std::get<2>( it ) );
	return std::nullopt;
}

/**
 * checkPos(long, long)
 * @brief Checks a given position for matches in the cache.
 * @param x								- Target horizontal X-axis position.
 * @param y								- Target vertical Y-axis position.
 * @return nullopt						- No entities are located at the given position.
 * @return pair<char, unsigned short>	- The display character & color of the entity located at the given position.
 */
std::optional<std::pair<char, unsigned short> > FrameBuffer::checkPos( const long x, const long y ) const noexcept { return checkPos( { x, y } ); }

/**
 * buildNextFrame(Coord)
 * @brief Returns a new frame of the entire cell
 * @param origin	- The top-left corner of the frame, as shown in the console screen buffer
 * @returns Frame
 */
Frame FrameBuffer::buildNextFrame( const Coord& origin )
{
	rebuildCache();
	std::vector<std::vector<char> > buffer;
	//try {
		buffer.reserve( _size._y );
		for ( auto y = 0; y < static_cast<signed>(buffer.capacity()); y++ ) {
			std::vector<char> row;
			row.reserve( _size._x );
			for ( auto x = 0; x < static_cast<signed>(row.capacity()); x++ ) {
				auto pos{ Coord( x, y ) };
				if ( _game.getTile( pos )->_isKnown ) {
					const auto entity{ checkPos( pos ) };
					if ( entity.has_value() )
						row.emplace_back( entity.value().first );
					else
						row.emplace_back( static_cast<char>(_game.getTile( pos )->_display) );
				}
				else
					row.emplace_back( ' ' );
			}
			buffer.emplace_back( row );
	}
	return Frame{ buffer, origin };
	//} catch ( ... ) { return {}; }
}

/**
 * display()
 * @brief (Re)Build the frame from current Gamespace data, and cleanup expired entities by calling Gamespace::cleanupDead().
 */
void FrameBuffer::display()
{
	// flush the stdout buffer
	fflush( stdout );
	// Remove dead actors
	_game.cleanupDead();
	// get a pointer to the game flare
	auto* flare{ _game.getFlare() };
	// Check if the frame is already initialized
	if ( _initialized ) {
		// flush the output buffer to prevent garbage characters from being displayed.
		// Get the new frame
		auto next = buildNextFrame( _origin );
		// iterate vertical axis (frame iterator targets cell coords, console iterator targets screen buffer coords)
		for ( long frameY{ 0 }, consoleY{ _origin._y }; frameY < static_cast<long>(next._frame.size()); frameY++, consoleY++ ) {
			// iterate horizontal axis for each vertical index
			for ( long frameX{ 0 }, consoleX{ _origin._x }; frameX < static_cast<long>(next._frame.at( frameY ).size()); frameX++, consoleX++ ) {
				sys::colorReset();
				// check if the tile at this pos is known to the player
				if ( _game.getTile( frameX, frameY )->_isKnown ) {
					const auto entity{ checkPos( frameX, frameY ) };
					if ( entity.has_value() ) {
						sys::cursorPos( consoleX * 2, consoleY );
						sys::colorSet( entity.value().second );
						printf( "%c", entity.value().first );
					}
						// Check if the game wants a screen color flare
					else if ( flare != nullptr && flare->pattern( frameX, frameY ) ) {
						// set the cursor position to target. (frameX is multiplied by 2 because every other column is blank space)
						sys::cursorPos( consoleX * 2, consoleY );
						if ( flare->time() % 2 == 0 && flare->time() != 1 ) {
							sys::colorSet( flare->color() );
							printf( "%c", next._frame.at( frameY ).at( frameX ) );
							sys::colorReset();
						}
						else
							printf( "%c", next._frame.at( frameY ).at( frameX ) );
					}
						// Selectively update each tile if this tile doesn't match the last frame.
					else if ( next._frame.at( frameY ).at( frameX ) != _last._frame.at( frameY ).at( frameX ) ) {
						sys::cursorPos( consoleX * 2, consoleY );
						printf( "%c", next._frame.at( frameY ).at( frameX ) );
					}
				}
					// Selectively update each unknown tile if this tile doesn't match the last frame.
				else if ( next._frame.at( frameY ).at( frameX ) != _last._frame.at( frameY ).at( frameX ) ) {
					sys::cursorPos( consoleX * 2, consoleY );
					printf( " " );
				}
			}
		}
		// set the last frame to this frame.
		_last = next;
		// Update the player stats box every other frame
		if ( _update_stats ) {
			// display the player stat bar
			_player_stats.display();
			_update_stats = false;
		}
		else
			_update_stats = true;
		// do flare functions
		if ( flare != nullptr ) {
			if ( flare->time() > 1 )
				flare->decrement();
			else
				_game.resetFlare();
		}
	}
	else initFrame(); // if the frame hasn't been initialized, initialize it.
}

/**
 * deinitialize()
 * @brief Set the initialized flag to false, causing the display to be re-initialized next time display() is called.
 */
void FrameBuffer::deinitialize() noexcept
{
	_initialized = false;
}
