/**
 * actor.h
 * Represents characters in the game.
 * Contains all actors used in the game.
 * by radj307
 */
#pragma once
#include <string>
#include "Coord.h"

// stats & specs of all actors -- parent struct
struct ActorBase {
	Coord _myPos;
	
	ActorBase(Coord& myPos) : _myPos(myPos) {}

	bool moveTo(Coord target)
	{

	}
};

// human player
class Player : public ActorBase {

public:
	Player(Coord myPos) : ActorBase(myPos) {}
};

// enemy actor
class Enemy : public ActorBase {

public:
	Enemy(Coord myPos) : ActorBase(myPos) {}
};