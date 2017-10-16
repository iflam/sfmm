#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();
    void *x = sf_malloc(16);
    /*void *g = */sf_malloc(16);
    void *y = sf_malloc(32);
    /*void *z = */sf_malloc(16);
    void *a = sf_malloc(32);
    void *b = sf_malloc(32);
    /*void *c = */sf_malloc(16);

    sf_free(b);
    sf_free(a);
    sf_free(y);
    sf_free(x);
    void *test = sf_malloc(32);
    sf_snapshot();
    sf_free(test);
    sf_mem_fini();
    return EXIT_SUCCESS;
}
