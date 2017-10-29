#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sfish.h"
char* makePrompt(char* cwd, char* homedir){
    char* occ;
    char* netid = "iflam";

    if((occ = strstr(cwd,homedir)) == NULL){
        return NULL;
    }
    occ += strlen(homedir)+1;
    char *returnChar = malloc(strlen(cwd) + strlen(netid) + 7);
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