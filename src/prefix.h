#ifndef __prefix_h
#define __prefix_h

#include <math.h>
#include <float.h>

#if !defined(DABES_IOS) && !defined(DABES_MAC)
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
  #define DABES_SDL
#endif

#include "constants.h"
#include "dbg.h"
#include "object.h"
#include "util.h"

extern char *bundlePath__;

#endif

#ifdef DABES_IOS
#define DABES_GLVERSION "gles"
#else
#define DABES_GLVERSION "gl"
#endif

#define shader_path(N) "shaders/" DABES_GLVERSION "/" N

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#ifdef __GNUC__
#  define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_ ## x
#else
#  define UNUSED_FUNCTION(x) UNUSED_ ## x
#endif
