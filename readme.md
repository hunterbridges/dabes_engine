DaBes Engine
============

_"I'm da besssss"_

What is done:
* Build targets:
    * Desktop (SDL, tested on OSX)
    * iOS
* Graphics engine:
    * Can load shaders and draw with them
    * Can load SDL surfaces into OpenGL textures
    * Camera tracking (Static, single entity, multi entity)
    * Can render TMX tile maps
    * Parallax layers
    * Sprite animations
* Physics engine:
    * Can use Chipmunk because writing a Physics engine is hard
    * Basic Hitboxes
    * Collision Sensors
* Audio engine:
    * OGG support
    * Seamless music queueing and looping
    * Sound effects
* Scripting engine:
    * Entity control scripts
    * Interaction scripts
    * Scene scripts

What is not yet done:
* Audio engine:
    * Spatialization
* Graphics engine:
    * Keyframe animations/Easer
    * HUD Layers
* Scripting engine:
    * Space bindings
    * Tile map bindings
    * Scene manager
* Debug tools:
    * Entity inspector

Bugs:
* Debug text currently dead
* Tile maps should be disposed of more thoroughly (destroy is leaking)
* Parallax zoom is sliiiightly weird
* Weird camera humping as tracked entity is rotating
* Getting "snagged" on ground tiles

Code Gripes:
* Too many cpp directives, not enough platform specific modules

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
[Learn C the Hard Way](http://c.learncodethehardway.org/book/) by Zed Shaw,
[Chipmunk Physics](http://chipmunk-physics.net/) by Howling Moon Software,
[Lua 5.2.1](http://www.lua.org/) by PUC-Rio


Asset Copyrights:
* Megaman is property of Capcom
* Ice Cap Zone is property of SEGA
* All other included graphics are public domain and are free for reuse
* All included music is Copyrighted to me, Hunter Bridges, and is not available for reuse
