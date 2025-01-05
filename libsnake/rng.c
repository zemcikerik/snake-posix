#include "rng.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

void init_rng() {
    srand48(time(NULL));
}

int uniform_dist(const int min, const int maxExclusive) {
    return (int) floor(drand48() * (maxExclusive - min) + min);
}
