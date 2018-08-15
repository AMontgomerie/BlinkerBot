# BlinkerBot
A bot for StarCraft II using C++ and [the StarCraft II API](https://github.com/Blizzard/s2client-api). The bot plays Protoss and focuses on producing stalkers and attacking using blink and kiting micro.

Features:
- Builds a proxy pylon, warps in stalkers and tries to attack aggressively using blink and kiting micro.
- Follows an initially scripted build order but then dynamically generates build order goals (in a limited way) following that.
- Continues to expand and add additional production facilities (currently only gateways and later adds robo for colossus).
- Researches warp gate, blink, and ground attack upgrades.
- Manages workers and expansions to maintain optimal saturation at each base.
- Defends against attacks and pulls workers to help the defence when necessary.
- Able to respond to cloaked units by producing detectors.