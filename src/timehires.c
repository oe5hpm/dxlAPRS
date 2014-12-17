
#include <time.h>

void gettime(struct timespec *secns)
{
  clock_gettime(CLOCK_MONOTONIC, (struct timespec *)secns);
}

