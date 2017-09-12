#include "p_cypher.h"
#include "hw1.h"
#include <stdio.h>

char findCol(char a, int col){
    char charToReturn = '\0';
    int i = 0;
    while(*(polybius_table+i) != a){
        if(*(polybius_table+i) == a){
            break;
        }
        i++;
    }
    i = i%col;
    if(i>9){
        charToReturn = i+101;
    }
    else{
        charToReturn = i+48;
    }
    return charToReturn;
}

char findRow(char a,int col){
    char charToReturn = '\0';
    int i = 0;
    int row = 0;
    while(*(polybius_table+i) != a){
        if(*(polybius_table+i) == a){
            break;
        }
        i++;
        if(i%col == 0 && col != 0){
            row++;
        }
    }
        charToReturn = row+48;
        if(row>9){
            charToReturn+=7;
        }
    return charToReturn;
}

char coordToSym(char row, char col, int totalCols){
    char charToReturn = '\0';
    if(row > 64){row-=7;}
    if(col > 64){col-=7;}
    int numToIncrement = (((int)row)-48)*totalCols+((int)col)-48;//0+row*totalCols+col
    charToReturn = *(polybius_table+numToIncrement);
    return charToReturn;
}
int p_encode(int row, int col){
    int end = 0;
    if(key != '\0'){
        puts("key exists");
        //KEY EXIST
        int i = 0; //I IS THE END OF THE KEY.
        for(i = 0; *(key+i) != '\0';i++){
            *(polybius_table+i) = *(key+i);
        }
        //start from position i
        int place = i;
        for(int k = i;k<row*col;k++){
            int isSame = 0;
            for(int l = 0; *(key+l)!= '\0';l++){
                if(*(polybius_alphabet+k-i) == *(key+l)){
                    isSame = 1;
                    break;
                }
            }
            if(isSame == 1){
                continue;
            }
            else{
                if(*(polybius_alphabet+place) == '\0'){end = 1;}
                if(end == 0){
                    *(polybius_table+place) = *(polybius_alphabet+k-i);
                }
                else{
                    *(polybius_table+place) = '\0';
                }
                place++;
            }
        }
    }
    else{
        //NO KEY
        for(int i = 0; i < row*col;i++){
            if(*(polybius_alphabet+i) == '\0'){end = 1;}
            if(end == 0){
                *(polybius_table+i) = *(polybius_alphabet+i);
            }
            else{
                *(polybius_table+i) = '\0';
            }
        }
    }
    printf("input:\n");
    char c;
    while((c = getchar()) != EOF){
        if(c == '\n'){
            putchar('\n');
            continue;
        }
        if(c == ' '){
            putchar(' ');
            continue;
        }
        else if(c == '\t'){
            putchar('\t');
            continue;
        }
        char colToPrint = findCol(c,col);
        char rowToPrint = findRow(c,col);
        putchar(rowToPrint);
        putchar(colToPrint);
    }
    return 0;
}

int p_decode(int row, int col){
    int end = 0;
    if(key != '\0'){
        puts("key exists");
        //KEY EXIST
        int i = 0; //I IS THE END OF THE KEY.
        for(i = 0; *(key+i) != '\0';i++){
            *(polybius_table+i) = *(key+i);
        }
        //start from position i
        int place = i;
        for(int k = i;k<row*col;k++){
            int isSame = 0;
            for(int l = 0; *(key+l)!= '\0';l++){
                if(*(polybius_alphabet+k-i) == *(key+l)){
                    isSame = 1;
                    break;
                }
            }
            if(isSame == 1){
                continue;
            }
            else{
                if(*(polybius_alphabet+place) == '\0'){end = 1;}
                if(end == 0){
                    *(polybius_table+place) = *(polybius_alphabet+k-i);
                }
                else{
                    *(polybius_table+place) = '\0';
                }
                place++;
            }
        }
    }
    else{
        //NO KEY
        for(int i = 0; i < row*col;i++){
            if(*(polybius_alphabet+i) == '\0'){end = 1;}
            if(end == 0){
                *(polybius_table+i) = *(polybius_alphabet+i);
            }
            else{
                *(polybius_table+i) = '\0';
            }
        }
    }
    //Do the checking
    printf("input:\n");
    char myRow;
    char myCol;
    char c;
    int decNum = 0;
    while((c = getchar()) != EOF){
        if(c == '\n'){
            putchar('\n');
            decNum = 0;
            continue;
        }
        if(c == ' '){
            putchar(' ');
            decNum = 0;
            continue;
        }
        else if(c == '\t'){
            putchar('\t');
            decNum = 0;
            continue;
        }
        if(decNum == 0){
            decNum = 1;
            myRow = c;
        }
        else{
            myCol = c;
            char symToPrint = coordToSym(myRow, myCol, col);
            putchar(symToPrint);
            decNum = 0;
        }
    }
    return 0;
}