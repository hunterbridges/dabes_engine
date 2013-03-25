DaBes Engine
============

_"I'm da besssss"_

What is done:
* Base layer of SDL/OpenGL
* Framework for decoupled graphics, physics, input, and audio engines
* Graphics engine:
    * Independent camera control (kinda sorta)
    * Can load shaders and draw with them
    * Can load SDL surfaces into OpenGL textures
    * Independent camera
* Physics engine:
    * Physics model that supports angular impulse collisions
    * Time scaling
    * Grid-based collision detection
* Audio engine:
    * Can play music and control the volume of it
* iOS build

What is not yet done:
* Remove SDL dependency on iOS
* Physics engine:
    * Raycasting
    * Raycast to build an accumulated opposing pressure
* Tile maps
* Parallax layers
* Scene management
* Object scripting

Bugs:
* Debug grid not working on desktop
* Debug text currently dead

```
# Ya gonna need dis stuf!
brew install --HEAD sdl sdl_image sdl_ttf sdl_mixer

make game
./game
```
