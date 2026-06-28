# gm_8bit - Parametric Voice Effects

An advanced real-time voice manipulation binary module for **Garry's Mod**. 
Originally a static voice filter, this fork introduces **Dynamic Parametric Audio Processing (EFF_CUSTOM)**, allowing developers to create infinite unique voices (Alien, Robot, Chipmunk, Radio, Demon) directly through Lua without recompiling!

## New Features
*  **EFF_CUSTOM**: A fully modular audio effect that you can tweak in real-time.
*  **Pitch Shift (Granular)**: Make voices squeaky like a child or deep like a monster.
*  **Lowpass Filter**: Perfect for realistic Combine masks, walkie-talkies, and walls.
*  **Reverb**: Add customizable echoes for caves, PA systems, and empty rooms.

## Lua API (How to use)
You can now create your own voice effects instantly using the new Lua bindings!

### Applying a Custom Voice
```lua
-- 1. Enable the custom effect channel for the player
eightbit.EnableEffect( player:UserID(), eightbit.EFF_CUSTOM )

-- 2. Configure the parameters:
-- eightbit.SetCustomEffect( userid, pitch, lowpass, reverb )
-- • pitch: 1.0 (normal), >1.0 (squeaky/alien), <1.0 (deep)
-- • lowpass: 0.0 (no filter), 0.5-0.9 (radio/mask effect)
-- • reverb: 0.0 (no echo), 0.1-0.9 (room/cave echo)

-- Example 1: Squeaky / Child voice
eightbit.SetCustomEffect( player:UserID(), 1.5, 0.0, 0.0 )

-- Example 2: Combine Radio (Deep + Muffled)
eightbit.SetCustomEffect( player:UserID(), 0.85, 0.7, 0.0 )

-- Example 3: Alien Cave Monster
eightbit.SetCustomEffect( player:UserID(), 0.6, 0.0, 0.6 )
```


---
*No more C++ compiling! Create infinite roleplay voices dynamically.*
