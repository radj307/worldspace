#pragma once
#include <string>

// stats & specs of all actors -- parent struct
struct ActorBase : public Coord {

	
	ActorBase(Coord& myPos) : Coord(myPos) {}
};

// human player
class Player : public ActorBase {

public:
	Player(Coord& myPos) : ActorBase(myPos) {}
};

// enemy actor
class Enemy : public ActorBase {

public:
	Enemy(Coord& myPos) : ActorBase(myPos) {}
};