#ifndef __constants_h
#define __constants_h

#ifdef DABES_IOS
#define FPS 60
#else
#define FPS 60
#endif

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define DEFAULT_PPM 32

#define NUM_BOXES 10

#define MVMT_RUN_ACCEL 15
#define MVMT_AIR_ACCEL 2 * MVMT_RUN_ACCEL
#define MVMT_TURN_ACCEL 3 * MVMT_RUN_ACCEL
#define MVMT_FRICTION MVMT_RUN_ACCEL
#define MVMT_MAX_VELO 15
#define MVMT_JUMP_VELO_HI -8
#define MVMT_JUMP_VELO_LO -4

#define PHYS_DEFAULT_GRID_SIZE 2.0
#define FIXTURE_HISTORY_LENGTH 10


#endif
