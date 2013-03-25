#ifndef __prefix_h
#define __prefix_h

#include <math.h>

#ifdef DABES_IOS
  #include <SDL.h>
#else
  #include <assert.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <limits.h>
  #include <lcthw/liblcthw.h>
  #include <SDL/SDL.h>
  #include <SDL/SDL_image.h>
  #include <SDL/SDL_opengl.h>
  #include <SDL/SDL_mixer.h>
  #include <SDL/SDL_ttf.h>
#endif

#include "constants.h"
#include "dbg.h"
#include "gfx_helpers.h"
#include "object.h"
#include "util.h"

extern char *bundlePath__;

#endif

#ifdef DABES_IOS
#define DABES_GLVERSION "gles"
#else
#define DABES_GLVERSION "gl"
#endif

#define shader_path(N) "media/shaders/" DABES_GLVERSION "/" N
