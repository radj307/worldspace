/**
 * game.h
 * Represents the game, with a worldspace & actors.
 * Contains the Gamespace class, which is a container for cells, and actors.
 * by radj307
 */
#pragma once
#include "cell.h"
#include "actor.h"

class Gamespace {
	Player _player;
	std::vector<Enemy> _hostile;

public:
	Cell &_world;

	Gamespace(Cell& worldspace) : _world(worldspace), _player(Coord(1, 1))
	{

	}
};