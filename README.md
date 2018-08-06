# BlinkerBot
A Protoss bot for SC2 in C++ using [SC2API](https://github.com/Blizzard/s2client-api). Includes blink micro and kiting, and simple dynamic build order generation after completing a scripted opening.

Features:
- Builds a forward pylon and warps in units to play aggressively.
- Stalkers with low shields blink away from enemies.
- Ranged units kite vs melee and shorter ranged units.
- When reaching the end of a scripted build order opening, the bot will dynamically generate new production goals (for example additional bases and production facilities).
