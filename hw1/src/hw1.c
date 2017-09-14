#include "hw1.h"
#include <stdlib.h>

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the program
 * and will return a unsigned short (2 bytes) that will contain the
 * information necessary for the proper execution of the program.
 *
 * IF -p is given but no (-r) ROWS or (-c) COLUMNS are specified this function
 * MUST set the lower bits to the default value of 10. If one or the other
 * (rows/columns) is specified then you MUST keep that value rather than assigning the default.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return Refer to homework document for the return value of this function.
 */
//validates to check that my arg is a number :D
int isNumber(int size, char* c){
    int myBool = 1;
    for(int i = 0; i < size ;i++){
        if(!(*(c+i) < 58 && *(c+i) > 47)){
            myBool = 0;
            break;
        }
    }
    return myBool;
}

short unsigned int convertToBinary(long int addVar){
    long binaryNumber = 0, place = 1;
    int remainder = 0;
    while(addVar !=0){
        remainder = addVar%2;
        addVar/=2;
        binaryNumber+=(remainder*place);
        place*=10;
    }
    return binaryNumber;
}

long int convertToNum(char* s){
    int num = 0;
    for(int i = 0; *s != '\0'; i++){
        num+=((int)(*s)-48);
        if(*(s+1) != '\0'){
            num*=10;
        }
        s++;
    }
    return num;
}
unsigned short validargs(int argc, char **argv) {
    //First we will find those args bruv. So, the first arg we need is *(argv+1), arg 2 is *(argv+2), etc.
    short unsigned returnValue = 0;
    char firstCommand =  *(*(argv+1)+1);
    //IF IS -h, 1000000000000000
    int alph_length = 0;
    if(firstCommand == 'h'){
        return 0x8000;
    }
    //IF IS -f, 01?00000????????
    else if(firstCommand == 'f'){
        returnValue = returnValue | 16384;
    }
    else if(firstCommand == 'p'){
        returnValue = 0;
        char* test_char = polybius_alphabet;
        while(*test_char != '\0'){
            alph_length++;
            test_char++;
        }
    }
    else{
        return 0;
    }
    char secondCommand = *(*(argv+2)+1); //should be d or e.
    //IF IS -d, 01100000????????
    if(secondCommand == 'd'){
        returnValue = returnValue | 8192;
    }
    else if(secondCommand == 'e'){
    }
    else{return 0;}
    //since it's 'd', add that one
    //returnValue == 11000000;//???? ???? are the remaining things to check
    //NOW CHECK IF THERE ARE ROWS/COLUMNS/KEY
    char thirdCommand = '\0', fourthCommand = '\0', fifthCommand = '\0';
    if(argc > 3){
        thirdCommand = *(*(argv+3)+1);
        if(argc > 5){
            fourthCommand = *(*(argv+5)+1);
            if(thirdCommand == fourthCommand){
                return 0;
            }
            if(argc > 7){
                fifthCommand = *(*(argv+7)+1);
                if(thirdCommand == fifthCommand){
                    return 0;
                }
                if(fourthCommand == fifthCommand){
                    return 0;
                }
            }
        }
    }
    char rCmd = '\0', cCmd = '\0', keyCmd = '\0';
    int rVal = -1, cVal = -1;
    char *keyVal = 0;
    if(thirdCommand != '\0'){
        if(thirdCommand == 'r'){
            rCmd = thirdCommand;
            rVal = (convertToNum(*(argv+4)));
        }
        else if (thirdCommand == 'c'){
            cCmd = thirdCommand;
            cVal = (convertToNum(*(argv+4)));
        }
        else if (thirdCommand == 'k'){
            keyCmd = thirdCommand;
            keyVal = *(argv+4);
        }
        else {return 0;}
    }
    if(fourthCommand != '\0'){
        if(fourthCommand == 'r'){
            rCmd = fourthCommand;
            rVal = (convertToNum(*(argv+6)));
        }
        else if (fourthCommand == 'c'){
            cCmd = fourthCommand;
            cVal = (convertToNum(*(argv+6)));
        }
        else if (fourthCommand == 'k'){
            keyCmd = fourthCommand;
            keyVal = *(argv+6);
        }
        else {return 0;}
    }
    if(fifthCommand != '\0'){
        if(fifthCommand == 'r'){
            rCmd = fifthCommand;
            rVal = (convertToNum(*(argv+8)));
        }
        else if (fifthCommand == 'c'){
            cCmd = fifthCommand;
            cVal = (convertToNum(*(argv+8)));
        }
        else if (fifthCommand == 'k'){
            keyCmd = fifthCommand;
            keyVal = *(argv+8);
        }
        else {return 0;}
    }
    //CHECK IF THERE IS -r
    if(rCmd > 0){
        if(rVal <9 || rVal >15){ //CHECK FOR INVALID ROWNUM
            return 0;
        }
        else{
            int addVal = rVal << 4;
            returnValue = returnValue | addVal; //adds that number to the binary number
        }
    }
    else{
        rVal = 10;
        returnValue = returnValue | 160;
    }
    if(cCmd > 0){
        if(cVal <9 || cVal >15){ //CHECK FOR INVALID ROWNUM
            return 0;
        }
        else{
            returnValue = returnValue | cVal; //adds that number to the binary number
        }
    }
    else{
        cVal = 10;
        returnValue = returnValue | 10;
    }
    if(keyCmd > 0){
        if(firstCommand == 'f'){
                char* current_char = keyVal;
                char prev_char = '\0';
                for(int i = 0; *current_char != '\0'; i++){
                    if(*current_char == prev_char){
                        return 0;
                    }
                    else{
                        int isSame = 0;
                        for(int k = 0; *(fm_alphabet+k) != '\0';k++){
                            int exists = 1; //FIX THIS!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~``
                            if(*current_char == *(fm_alphabet+k)){
                                isSame = 1;
                                break;
                            }
                        }
                        if(isSame == 0){
                            return 0;
                        }
                    }
                    prev_char = *current_char;
                    current_char = current_char+1;
                }
                key = keyVal;
        }
        else if(firstCommand == 'p'){
            extern char* polybius_alphabet;
            char* current_char = keyVal;
                char prev_char = '\0';
                for(int i = 0; *current_char != '\0'; i++){
                    if(*current_char == prev_char){
                        return 0;
                    }
                    else{
                        int isSame = 0;
                        for(int k = 0; *(polybius_alphabet+k) != '\0';k++){
                            int exists = 1; //FIX THIS!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~``
                            if(*current_char == *(polybius_alphabet+k)){
                                isSame = 1;
                                break;
                            }
                        }
                        if(isSame == 0){
                            return 0;
                        }
                    }
                    prev_char = *current_char;
                    current_char = current_char+1;
                }
                key = keyVal;
        }
    }
    else{
        key = '\0';
    }
    if(cVal*rVal < alph_length){
        return 0;
    }
    return returnValue;
}
