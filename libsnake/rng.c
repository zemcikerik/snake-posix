#include "rng.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

void rng_init() {
    srand48(time(NULL));
}

int rng_uniform_dist(const int min, const int maxExclusive) {
    return (int) floor(drand48() * (maxExclusive - min) + min);
}
