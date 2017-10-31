#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sfish.h"
char* makePrompt(char* cwd, char* homedir){
    char* occ;
    char* netid = "iflam";

    if((occ = strstr(cwd,homedir)) == NULL){
        char *returnChar = malloc(strlen(cwd) + strlen(netid) + 7);
        returnChar = strcat(returnChar, cwd);
        returnChar = strcat(returnChar, " :: ");
        returnChar = strcat(returnChar, netid);
        returnChar = strcat(returnChar, " >> ");
        return returnChar;
    }
    occ += strlen(homedir);
    char* returnChar = malloc(strlen(cwd) + strlen(netid) + 7);
    memset(returnChar,0,sizeof(*returnChar));
    returnChar = strcat(returnChar, "~");
    returnChar = strcat(returnChar, occ);
    returnChar = strcat(returnChar, " :: ");
    returnChar = strcat(returnChar, netid);
    returnChar = strcat(returnChar, " >> ");
    return returnChar;
}

token* tokenify(token* pt, char* val){
    token* currentToken;
    token* prevToken;
    char* currentVal;;
    prevToken = pt;
    currentVal = val;
    currentToken = malloc(sizeof(token));
    if(prevToken == NULL || prevToken->type == PIPE){
        currentToken->type = PROG;
        currentToken->contents = val;
        currentToken->next = NULL;
        if(prevToken != NULL)
            prevToken->next = currentToken;
    }

    else if(prevToken->type == LRED){
        currentToken->type = IN;
        currentToken->contents = val;
        currentToken->next = NULL;
        if(prevToken != NULL)
            prevToken->next = currentToken;
    }

    else if(prevToken->type == RRED){
        currentToken->type = OUT;
        currentToken->contents = val;
        currentToken->next = NULL;
        if(prevToken != NULL)
            prevToken->next = currentToken;
    }

    else if(strcmp(currentVal, "|") == 0){
        currentToken->type = PIPE;
        currentToken->contents = "|";
        currentToken->next = NULL;
        if(prevToken != NULL)
            prevToken->next = currentToken;
    }

    else if(strcmp(currentVal, "<") == 0){
        currentToken->type = LRED;
        currentToken->contents = "<";
        currentToken->next = NULL;
        if(prevToken != NULL)
            prevToken->next = currentToken;
    }

    else if(strcmp(currentVal, ">") == 0){
        currentToken->type = RRED;
        currentToken->contents = ">";
        currentToken->next = NULL;
        if(prevToken != NULL)
            prevToken->next = currentToken;
    }

    else{
        currentToken->type = ARG;
        currentToken->contents = val;
        currentToken->next = NULL;
        if(prevToken != NULL)
            prevToken->next = currentToken;
    }

    return currentToken;
}

token* makeTokens(char* input){
    token* firstToken = NULL;
    token* currentToken = NULL;
    token* prevToken = NULL;
    char* currentVal = NULL;
    if((currentVal = strtok(input," "))!= NULL){
        firstToken = tokenify(prevToken, currentVal);
        prevToken = firstToken;
    }
    while((currentVal = strtok(NULL," "))!= NULL){
        currentToken = tokenify(prevToken, currentVal);
        prevToken = currentToken;
    }
    return firstToken;
}

// struct program {
//     ProgType programType;
//     char* name;
//     char* args;
//     char* infile;
//     char* outfile;
//     IoType* itype;
//     IoType* otype;
//     program* next;
// }
program* setEmpty(program* p){
    p->programType = PROGRAM;
    p->name = NULL;
    p->args = NULL;
    p->infile = NULL;
    p->outfile = NULL;
    p->itype = NONE;
    p->otype = NONE;
    p->next = NULL;
    return p;
}

program* parsify(program* pp, token* ft){
    program* firstProgram = malloc(sizeof(program));
    setEmpty(firstProgram);
    program *prevProgram = pp;
    token *firstToken = ft;
    token* currentToken = firstToken;
    token* firstArg;
    token* currentArg;
    char** args;
    bool hasRred = false;
    bool hasLred = false;
    //first program
    while(currentToken != NULL){
        switch(currentToken->type){
            case IN:
                if(hasLred){
                    firstProgram->programType = BAD;
                    firstProgram->itype = LRED1;
                    return firstProgram;
                }
                firstProgram->infile = currentToken->contents;
                firstProgram->itype = LRED1;
                hasLred = true;
                break;
            case OUT:
                if(hasRred){
                    firstProgram->programType = BAD;
                    firstProgram->itype = RRED1;
                    return firstProgram;
                }
                firstProgram->outfile = currentToken->contents;
                firstProgram->otype = RRED1;
                hasRred = true;
                break;
            case RRED:
                if(currentToken->next == NULL){
                    firstProgram->programType = BAD;
                    firstProgram->itype = NULLTYPE;
                    return firstProgram;
                }
                break;
            case LRED:
                if(currentToken->next == NULL){
                    firstProgram->programType = BAD;
                    firstProgram->itype = NULLTYPE;
                    return firstProgram;
                }
                break;
            case PIPE:
                if(currentToken->next == NULL){
                    firstProgram->programType = BAD;
                    firstProgram->itype = NULLTYPE;
                    return firstProgram;
                }
            default:
                break;
        }
        currentToken = currentToken->next;
    }
    currentToken = firstToken;
    if((strcmp(currentToken->contents,"help")) == 0){
        firstProgram->programType = HELP;
        //Implement redirection later
        if(currentToken->next != NULL){
            if(currentToken->next->type == RRED){
                currentToken = currentToken->next;
                if(currentToken->next != NULL){
                    firstProgram->otype = RRED;
                    firstProgram->outfile =currentToken->next->contents;
                }
            }
        }
        return firstProgram;
    }
    else if((strcmp(currentToken->contents,"cd")) == 0){
        firstProgram->programType = CD;
    }
    else if((strcmp(currentToken->contents,"exit")) == 0){
        firstProgram->programType = EXIT;
        return firstProgram;
    }
    else if((strcmp(currentToken->contents,"pwd")) == 0){
        firstProgram->programType = PWD;
        //implement redirection later
        if(currentToken->next != NULL){
            if(currentToken->next->type == RRED){
                currentToken = currentToken->next;
                if(currentToken->next != NULL){
                    firstProgram->otype = RRED;
                    firstProgram->outfile =currentToken->next->contents;
                }
            }
        }
        return firstProgram;
    }
    else{
        firstProgram->programType = PROGRAM;
    }

    int numArgs = 0;
    firstArg = currentToken;
    currentArg = firstArg;
    while(currentArg != NULL){
        if(currentArg->type == PIPE){
            break;
        }
        if(currentArg->type == RRED){
            break;
        }
        if(currentArg->type == LRED){
            break;
        }
        numArgs++;
        currentArg = currentArg->next;
    }
    currentArg = firstArg;
    //create a pointer that points to char pointers
    args = malloc((numArgs+1)*sizeof(char*)); //the amount of char pointers will be how many arguments, plus one for NULL.
    for(int k = 0; k < numArgs; k++){
        *(args+k) = malloc(strlen(currentArg->contents)+1);
        strcpy(*(args+k),currentArg->contents);
        currentArg = currentArg->next;
    }
    *(args+numArgs) = malloc(sizeof(NULL));
    *(args+numArgs) = (char*)NULL;
    firstProgram->args = args;
    firstProgram->name = *args;
    if(prevProgram != NULL){
        prevProgram->next = firstProgram;
    }
    return firstProgram;
}

program* parseTokens(token* firstToken){
    program* firstProgram = malloc(sizeof(program));
    program* currentProgram;
    program* prevProgram;
    setEmpty(firstProgram);
    firstProgram = parsify(NULL, firstToken);
    token* currentArg = firstToken;
    prevProgram = firstProgram;
    while(currentArg != NULL){
        if(currentArg->type == PIPE){
            currentProgram = malloc(sizeof(program));
            setEmpty(currentProgram);
            currentProgram = parsify(prevProgram, currentArg->next);
            prevProgram = currentProgram;
        }
        if(currentProgram->programType == BAD){
            return currentProgram;
        }
        currentArg = currentArg->next;
    }
    return firstProgram;
}

