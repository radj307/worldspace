# worldspace
    Modular console-based game that uses a matrix of Tile objects as its world.
    Currently contains actors with basic pathing, items, events, a double-buffered frame display system to prevent flickering, and many other features.
    
    The game itself can be customized by changing the members of the GameRules struct after instantiating it.
    
    Uses multithreading, with mutexes & atomic variables.
