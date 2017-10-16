#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

    sf_mem_init();

    void *x = sf_malloc(sizeof(int));
    /* void *y = */ sf_malloc(10);
    x = sf_realloc(x, sizeof(int) * 10);
    sf_free(x);
    sf_mem_fini();
    puts("finished");
    return EXIT_SUCCESS;
}
