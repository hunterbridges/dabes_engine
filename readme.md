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
    * Camera tracking
    * Can render TMX tile maps
    * Parallax layers
    * Sprite animations
* Physics engine:
    * Can use Chipmunk because writing a Physics engine is hard
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
    * Source volume
* Graphics engine:
    * Keyframe animations
* Hitboxes
* Scripting engine:
    * Camera bindings
    * Space bindings
    * Sfx bindings
    * Tile map bindings
    * Scene manager
* Debug tools:
    * Entity inspector

Bugs:
* Debug text currently dead
* Tile maps should be disposed of more thoroughly (destroy is leaking)
* Parallax zoom is sliiiightly weird
* Weird camera humping as tracked entity is rotating
* Audio buffers getting orphaned somehow?
* Getting stuck in ground tiles
* Lua GC is not playing nice
* can_rotate in Body_create seems to not work

Code Gripes:
* Too many cpp directives, not enough platform specific modules

```
# Ya gonna need dis stuf!
brew install --HEAD sdl sdl_image sdl_ttf sdl_mixer libogg libvorbis

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
