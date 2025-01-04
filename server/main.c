#include <stdio.h>
#include "../librng/rng.h"

int main() {
    printf("Hello, World! %d\n", uniform_dist_glb(2, 3));
    return 0;
}
