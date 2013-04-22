DaBes Engine
============

_"I'm da besssss"_

What is done:
* Base layer of SDL/OpenGL
* Framework for decoupled graphics, physics, input, and audio engines
* Build for desktop (with SDL) or iOS
* Graphics engine:
    * Can load shaders and draw with them
    * Can load SDL surfaces into OpenGL textures
    * Camera tracking
    * Can render TMX tile maps
    * Parallax layers
* Physics engine:
    * Can use Chipmunk because writing a Physics engine is hard
* Audio engine:
    * Can play music and control the volume of it

What is not yet done:
* Graphics engine:
    * Sprite animations
* Audio engine:
    * Sound effects
* Object inspector
* Object scripting

Bugs:
* Debug text currently dead
* Tile maps should be disposed of more thoroughly (destroy is leaking)
* Gap in audio loop, gotta use smaller format audio files...
* Parallax zoom and rotate

Code Gripes:
* Too many cpp directives, not enough platform specific modules

```
# Ya gonna need dis stuf!
brew install --HEAD sdl sdl_image sdl_ttf sdl_mixer

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
