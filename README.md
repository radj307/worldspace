# worldspace
This is a game/concept I made for fun in 2020.  

Unfortunately, I didn't save copies of the original versions of my personal libraries _(now `307lib`)_, so I ported it to CMake.

## Features
- Console-Based Roguelike ASCII RPG
- Live-Action using multithreading and non-blocking console input.
- Uses a double-buffered, selective-update-frame-comparison technique to prevent flicker and make the game more responsive.
- An assortment of NPCs, enemies, traps, consumables, and a boss battle event with randomly-selected boss enemies.
- Enemies have basic pathfinding, detection ranges, and stats.
- Highly configurable.
  - Configurable "framerate"
  - Configurable keybinds
  - Enemies are created using `.ini` configs that define their stats & appearance.
