#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();
    void *x = sf_malloc(sizeof(double) * 8);
    void *y = sf_realloc(x, PAGE_SZ);
    void *z = sf_malloc(10);
    void *a = sf_realloc(y, PAGE_SZ * 2);

    sf_free(z);
    sf_free(a);
    sf_free(y);
    sf_free(x);
    sf_snapshot();
    sf_mem_fini();
    return EXIT_SUCCESS;
}
