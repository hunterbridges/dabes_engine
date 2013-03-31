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
    * Independent camera
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
