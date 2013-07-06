#ifndef __util_h
#define __util_h
#include "prefix.h"
#define sign(N) (N > 0) - (N < 0)

#ifndef MIN
#define MIN(A, B) (A < B ? A : B)
#endif
#ifndef MAX
#define MAX(A, B) (A > B ? A : B)
#endif

#define countfunc() \
  static int func_ctr = 0; \
  func_ctr++; \
  printf("%s called %d\n", __func__, func_ctr);

#define fequal(a,b) (fabs((a) - (b)) < FLT_EPSILON)
#define streq(A, B) (strcmp((const char *)A, (const char *)B) == 0)

#endif
