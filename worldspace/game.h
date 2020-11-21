#pragma once
#include "world.h"
#include "actor.h"

class Gamespace : public Cell {


public:
	Gamespace(Coord cellSize, bool showAllTiles = false) : Cell(cellSize, showAllTiles) {}
};