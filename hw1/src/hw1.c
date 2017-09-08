#include "hw1.h"

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
        printf("char %d: %c\n",i,*(c+i));
        if(!(*(c+i) < 58 && *(c+i) > 47)){
            myBool = 0;
            break;
        }
    }
    return myBool;
}

long int convertToBinary(long int var,long int addVar){
    while(addVar >= 1){
        if(addVar!=1){
            var += addVar%2;
            var*=10;
            addVar/=2;
        }
        else{
            var += addVar%2;
            addVar/=2;
        }
    }
    return var;
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
    char firstCommand =  *(*(argv+1)+1);
    printf("First Command: %c\n", firstCommand);
    //IF IS -h, 1000000000000000
    if(firstCommand == 'h'){
        return 0x8000;
    }
    //IF IS -f, 01?00000????????
    else if(firstCommand == 'f'){
        long int returnValue = 10; //APPEND A 10
        char secondCommand = *(*(argv+2)+1); //should be d or e.
        printf("Second Command: %c\n", secondCommand);
        //IF IS -d, 01100000????????
        if(secondCommand == 'd'){
            returnValue = 11000000;
            //since it's 'd', add that one
            //returnValue == 11000000;//???? ???? are the remaining things to check
            //NOW CHECK IF THERE ARE ROWS/COLUMNS/KEY
            char thirdCommand = '\0', fourthCommand = '\0', fifthCommand = '\0';
            if(argc > 3){
                thirdCommand = *(*(argv+3)+1);
                printf("Third Command: %c\n", thirdCommand);
                if(argc > 5){
                    fourthCommand = *(*(argv+5)+1);
                    printf("Fourth Command: %c\n", fourthCommand);
                    if(thirdCommand == fourthCommand){
                        printf("cmd 3 and 4 are the same\n");
                        return 0;
                    }
                    if(argc > 7){
                        fifthCommand = *(*(argv+7)+1);
                        printf("Fifth Command: %c\n", fifthCommand);
                        if(thirdCommand == fifthCommand){
                            printf("cmd 3 and 5 are the same\n");
                            return 0;
                        }
                        if(fourthCommand == fifthCommand){
                            printf("cmd 4 and 5 are the same\n");
                            return 0;
                        }
                    }
                }

            }
            char rCmd = '\0', cCmd = '\0', keyCmd = '\0';
            char *rVal = '\0', *cVal = '\0', *keyVal = '\0';
            if(thirdCommand != '\0'){
                if(thirdCommand == 'r'){
                    printf("third command is r\n");
                    rCmd = thirdCommand;
                    rVal = (*(argv+4));
                    printf("command arg is %s\n",rVal);
                }
                else if (thirdCommand == 'c'){
                    printf("third command is c\n");
                    cCmd = thirdCommand;
                    cVal = (*(argv+4));
                    printf("command arg is %s\n",cVal);
                }
                else if (thirdCommand == 'k'){
                    printf("third command is k\n");
                    keyCmd = thirdCommand;
                    keyVal = (*(argv+4));
                    printf("command arg is %s\n",keyVal);
                }
                else {return 0;}
            }
            if(fourthCommand != '\0'){
                if(fourthCommand == 'r'){
                    printf("fourth command is r\n");
                    rCmd = fourthCommand;
                    rVal = (*(argv+6));
                    printf("command arg is %s\n",rVal);
                }
                else if (fourthCommand == 'c'){
                    printf("fourth command is c\n");
                    cCmd = fourthCommand;
                    cVal = (*(argv+6));
                    printf("command arg is %s\n",cVal);
                }
                else if (fourthCommand == 'k'){
                    printf("fourth command is k\n");
                    keyCmd = fourthCommand;
                    keyVal = (*(argv+6));
                    printf("command arg is %s\n",keyVal);
                }
                else {return 0;}
            }
            if(fifthCommand != '\0'){
                if(fifthCommand == 'r'){
                    printf("fifth command is r\n");
                    rCmd = fifthCommand;
                    rVal = (*(argv+8));
                    printf("command arg is %s\n",rVal);
                }
                else if (fifthCommand == 'c'){
                    printf("fifth command is c\n");
                    cCmd = fifthCommand;
                    cVal = (*(argv+8));
                    printf("command arg is %s\n",cVal);
                }
                else if (fifthCommand == 'k'){
                    printf("fifth command is k\n");
                    keyCmd = fifthCommand;
                    keyVal = (*(argv+8));
                    printf("command arg is %s\n",keyVal);
                }
                else {return 0;}
            }
            *rVal = *cVal + *keyVal;
            //CHECK IF THERE IS -r
            if(rCmd != '\0'){
                if((int)rCmd<9 || (int)rCmd>15){ //CHECK FOR INVALID ROWNUM
                    return 0;
                }



            }
            if(cCmd != '\0'){


            }
            if(keyCmd != '\0'){


            }

            //IF -r, CHECK THE VALUE.
            // char thirdCommand = *(*(argv+3)+1); //value for potential -r or -c
            // if(thirdCommand == 'r'){ //it is -r
            //     char* fourthCommand = *(argv+4); //check if this next arg is a number
            //     int sizeOfArg = 0;
            //     while(*fourthCommand != '\0'){
            //         sizeOfArg++;
            //         fourthCommand++;
            //     }
            //     if(isNumber(sizeOfArg, fourthCommand)){
            //         //it is a number, ie the user specified row number.
            //         if((int)fourthCommand < 9 || (int)fourthCommand > 15){ //CHECK FOR INVALID ROWNUM
            //             return 0;
            //         }
            //         else{
            //             int rowNum = convertToNum(fourthCommand); //converts it into a number
            //             returnValue = convertToBinary(returnValue, rowNum); //adds that number to the binary number
            //         }
            //     }
            //     else{return 0;} //THE USER DID NOT INPUT AN AMOUNT OF ROWS!
            // }
            // else if(thirdCommand == 'c'){ //

            // }
        }
        //IF IS -e, 01000000????????
        return 0;
    }
    else{
        return 0;
    }
}
