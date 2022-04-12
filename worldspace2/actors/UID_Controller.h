#pragma once

// Actor ID controller object
static struct {
private:
	ID currentID{ SHRT_MIN };

public:
	ID getID() { return ++currentID; }
} UID_Controller;
