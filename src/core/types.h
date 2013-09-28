#ifndef dab_types_h
#define dab_types_h

typedef int8_t dab_int8;
typedef uint8_t dab_uint8;
typedef int16_t dab_int16;
typedef uint16_t dab_uint16;
typedef int32_t dab_int32;
typedef uint32_t dab_uint32;
typedef int64_t dab_int64;
typedef uint64_t dab_uint64;

typedef dab_int8 dab_short;
typedef dab_uint8 dab_ushort;
typedef dab_int16 dab_int;
typedef dab_uint16 dab_uint;
typedef dab_int32 dab_long;
typedef dab_uint32 dab_ulong;

typedef int8_t dab_char;
typedef uint8_t dab_uchar;

#ifdef DABES_SDL
typedef float dab_float;
typedef double dab_double;
#else
typedef Float32 dab_float;
typedef Float64 dab_double;
#endif

#endif
