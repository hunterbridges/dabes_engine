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
* Physics engine:
    * Physics model that supports angular impulse collisions
    * Time scaling
* Audio engine:
    * Can play music and control the volume of it

What is not yet done:
* Inter-entity collision detection
* Automatic camera
* Parallax layers
* High-level state management
* Object scripting
* I guess some kind of level editor

```
# Ya gonna need dis stuf!
brew install --HEAD sdl sdl_image sdl_ttf sdl_mixer

make game
./game
```
