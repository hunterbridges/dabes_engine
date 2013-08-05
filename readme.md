DaBes Engine
============

_"I'm da besssss"_

Features:
* Build targets:
    * Desktop (SDL, tested on OSX)
    * iOS
* Graphics engine:
    * Render TMX tile maps
    * Parallax layers
    * Camera tracking (Static, single entity, multi entity)
    * Sprite animations
    * Keyframe animations/Easer
    * Static scenes
    * Use Freetype to render strings with TTF and OTF fonts
    * Overlay layers
* Physics engine:
    * Integration with Chipmunk 2D physics engine
    * Basic Hitboxes
    * Collision Sensors
    * Entity recording and playback
* Audio engine:
    * OGG support
    * Seamless music queueing and looping
    * Sound effects
* Scripting engine:
    * Lua scripting interface for most engine features
    * Scene manager

TODOs:
* Audio - spatialization/panning
* Scripting - space bindings
* Scripting - tile map bindings
* Overlay - entity tracking

Bugs:
* Tile maps should be disposed of more thoroughly (destroy is leaking)
* Parallax zoom is sliiiightly weird
* Weird camera humping as tracked entity is rotating
* Easers have a tendency to introduce leaks

```
# Ya gonna need dis stuf!
brew install sdl sdl_image sdl_ttf sdl_mixer libogg libvorbis

# Also install OpenAL Soft
# http://kcat.strangesoft.net/openal.html

make submodules
make game
./game
```

If you are trying to build the XCode project, you need to build the submodule
```
make submodules
```

Acknowledgements:

[Learn C the Hard Way](http://c.learncodethehardway.org/book/) by Zed Shaw

[Chipmunk Physics](http://chipmunk-physics.net/) by Howling Moon Software

[Lua 5.2.1](http://www.lua.org/) by PUC-Rio



Asset Copyrights:
* Megaman is property of Capcom
* Ice Cap Zone is property of SEGA
* All other included graphics are public domain and are free for reuse
* All included music is Copyrighted to me, Hunter Bridges, and is not available for reuse

