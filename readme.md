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
* Physics engine:
    * Physics model that supports angular impulse collisions
    * Time scaling
    * Grid-based collision detection
    * Can glitchingly collide with tile maps
* Audio engine:
    * Can play music and control the volume of it

What is not yet done:
* Graphics engine:
    * Parallax layers
    * Sprite animations
* Audio engine:
    * Sound effects
* Physics engine:
    * Raycasting
    * Raycast to build an accumulated opposing force
* Object inspector
* Object scripting

Bugs:
* Debug text currently dead
* Edge collisions not calculating properly
* Tile maps should be disposed of more thoroughly (destroy is leaking)
* Tiles are sliiiiiightly offset
* Game entity movement jumpy when at high coordinate values (Far right, for example)
* Desktop/SDL build is kinda unstable about textures
* Why is desktop tile val stored in .b? (gs/tilemap.frag)
* Gap in audio loop
* Gotta use smaller format audio files...

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
