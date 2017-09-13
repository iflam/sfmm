#include "f_cypher.h"
#include "hw1.h"

int compareStrings(char* sOne, char* sTwo){
    int same = 1;
    for(int i = 0; *(sOne+i) != '\0'; i++){
        if(*(sOne+i) != *(sTwo+i)){
            same = 0;
        }
    }
    return same;
}

void clearBuffer(char* buffer){
    for(int i = 0; i < 6; i++){
        *(buffer+i) = '\0';
    }
}

void findKey(char* buffer){
    for(int i = 0; *(fractionated_table+i) != '\0';i++){
        char* table_val = (char*)(*(fractionated_table+i));
        if(compareStrings(table_val,buffer) == 1){
            putchar(*(fm_key+i));
        }
    }
}

int f_encode(){
    int end = 0;
    if(key != '\0'){
        //KEY EXISTS
        int i = 0; //I IS THE END OF THE KEY.
        for(i = 0; *(key+i) != '\0';i++){
            *(fm_key+i) = *(key+i);
        }
        //start from position i
        int place = i;
        for(int k = i;*(fm_alphabet+k-i) != '\0'; k++){
            int isSame = 0;
            for(int l = 0; *(key+l)!= '\0';l++){
                if(*(fm_alphabet+k-i) == *(key+l)){
                    isSame = 1;
                    break;
                }
            }
            if(isSame == 1){
                continue;
            }
            else{
                if(*(fm_alphabet+place) == '\0'){end = 1;}
                if(end == 0){
                    *(fm_key+place) = *(fm_alphabet+k-i);
                }
                else{
                    *(fm_key+place) = '\0';
                }
                place++;
            }
        }

    }
    else{
        for(int i = 0; *(fm_alphabet + i) != '\0'; i++){
            *(fm_key+i) = *(fm_alphabet+i);
        }
    }
    for(int i = 0; *(fm_key + i) != '\0'; i++){
        printf("%c",*(fm_key+i));
    }
    puts("");
    puts("input: ");
    char c;
    long space;
    char *buffer = (char*)&space;
    int prevSpace = 1;
    clearBuffer(buffer);
    int bufferSize = 0;
    while((c = getchar()) != EOF){
        char *ptr = (char*)(*(morse_table+c-33));
        if(c == '\n'){
            *(buffer+bufferSize++) = 'x';
            if(bufferSize >= 3){
                findKey(buffer);
            }
            clearBuffer(buffer);
            bufferSize = 0;
            continue;
        }
        if(c == ' '){
            if(prevSpace == 0){
                prevSpace = 1;
                *(buffer+bufferSize++) = 'x';
                if(bufferSize >= 3){
                    findKey(buffer);
                    clearBuffer(buffer);
                    if(bufferSize >=4){
                        bufferSize = 0;
                        *(buffer+bufferSize++) = 'x';
                    }
                    bufferSize = 0;
                }
            }
            continue;
        }
        while(*ptr!= '\0'){
            prevSpace = 0;
            while(*(buffer+bufferSize) != '\0'){
                bufferSize++;
            }
            *(buffer+bufferSize) = *ptr;
            bufferSize++;
            //ABOVE SHOULD PUT PTR INTO BUFFER
            if(bufferSize >= 3){ //FIND THE RIGHT KEY PEICE
                findKey(buffer);
                clearBuffer(buffer);
                bufferSize = 0;
            }
            ptr++;
            if(c == '\n'){
                break;
            }
        }
        *(buffer+bufferSize++) = 'x';
        if(bufferSize >= 3){
            findKey(buffer);
            clearBuffer(buffer);
            bufferSize = 0;
        }
    }
    return 0;
}

int f_decode(){
    return 0;
}