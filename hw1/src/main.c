#include <stdlib.h>

#include "hw1.h"
#include "debug.h"
#include "p_cypher.c"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    unsigned short mode;
    int good_cypher;
    mode = validargs(argc, argv);

    debug("Mode: 0x%X", mode);

    if(mode & 0x8000) {
        USAGE(*argv, EXIT_SUCCESS);
    }

    else if(mode == 0){
        USAGE(*argv,EXIT_FAILURE);
    }
    else{
        int row = 0;
        int col = 0;
        row = mode & 0x00F0;
        row = row >> 4;
        col = mode & 0x000F;
        printf("row: %d col: %d \n",row,col);
        if(mode & 0x4000){
            if(mode & 0x2000){
                puts("-f -d");

            }
            else{
                puts("-f -e");
            }
        }
        else{
            if(mode & 0x2000){
                puts("-p -d");
                good_cypher = p_cypher(row,col);
            }
            else{
                puts("-p -e");
            }
        }
        if(good_cypher){
            puts("coolio");
        }
    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */