#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "sfish.h"
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
job *head;
job *currentJob;
int jobNum;
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

// char** makeArgs(){

// }

program* programify(char* input){
    program* currProg = malloc(sizeof(program));
    setEmpty(currProg);
    TokenType tok = PROG;
    char* currVal = malloc((strlen(input)+1)*sizeof(char*));
    memset(currVal,0,sizeof(char));
    char* currChar = input;
    char** args = malloc((strlen(input) + 1)*sizeof(char*));
    memset(args,0,sizeof(char));
    int argNum = 0;
    bool hasRed = false;
    while(*currChar != '\0'){
        while(*currChar == ' '){
            currChar++;
        }
        if(*currChar == '\0'){
            break;
        }
        while(*currChar != ' ' && *currChar != '\0' && *currChar != '<' && *currChar != '>'){
            strncat(currVal, currChar,1);
            currChar++;
        }
        switch(tok){
            case PROG:
            //make it the program one
                currProg->name = strdup(currVal);
                args[argNum] = strdup(currVal);
                argNum++;
                if((strcmp(currVal,"help"))==0){
                    currProg->programType = HELP;
                }
                else if((strcmp(currVal,"cd"))==0){
                    currProg->programType = CD;
                }
                else if((strcmp(currVal,"pwd"))==0){
                    currProg->programType = PWD;
                }
                else if((strcmp(currVal,"exit"))==0){
                    exit(3);
                }
                else if((strcmp(currVal,"jobs"))==0){
                    currProg->programType = JOBS;
                }
                else if((strcmp(currVal,"fg"))==0){
                    currProg->programType = FG;
                }
                else if((strcmp(currVal,"kill"))==0){
                    currProg->programType = KILL;
                }
                else{
                    currProg->programType = PROGRAM;
                }
                while(*currChar == ' '){
                    currChar++;
                }
                switch(*currChar){
                    case '<':
                        tok = IN;
                        currChar++;
                        break;
                    case '>':
                        tok = OUT;
                        currChar++;
                        break;
                    default:
                        tok = ARG;
                        break;
                }
                break;
            case ARG:
                    if((strcmp(currVal,"")) != 0){
                        args[argNum] = strdup(currVal);
                        argNum++;
                    }
                switch(*currChar){
                    case '<':
                        tok = IN;
                        currChar++;
                        break;
                    case '>':
                        tok = OUT;
                        currChar++;
                        break;
                    default:
                        break;
                }
                break;
            case IN:
                currProg->infile = strdup(currVal);
                currProg->itype = LRED1;
                while(*currChar == ' '){
                    currChar++;
                }
                switch(*currChar){
                    case '<':
                        currProg->programType = BAD;
                        currProg->itype = LRED1;
                        return currProg;
                        break;
                    case '>':
                        if(hasRed){
                            currProg->programType = BAD;
                            currProg->itype = RRED2;
                        }
                        tok = OUT;
                        hasRed = true;
                        currChar++;
                        break;
                    default:
                        break;
                }
                break;
            case OUT:
                currProg->outfile = strdup(currVal);
                currProg->otype = RRED1;
                while(*currChar == ' '){
                    currChar++;
                }
                switch(*currChar){
                    case '<':
                        if(hasRed){
                            currProg->programType = BAD;
                            currProg->itype = LRED2;
                        }
                        tok = IN;
                        hasRed = true;
                        currChar++;
                        break;
                    case '>':
                        currProg->programType = BAD;
                        currProg->itype = RRED1;
                        return currProg;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        memset(currVal,0,sizeof(char));
        if(*currChar == '\0'){
            break;
        }
    }
    args[argNum] = '\0';
    currProg->args = args;
    return currProg;
}
program* makePrograms(char* input){
    char* currentProg;
    program* firstProgram;
    program* prevProgram;
    program* currentProgram;
    if((currentProg = strtok(input,"|")) == NULL){
        //NO PIPES
        currentProg = input;
    }
    firstProgram = programify(currentProg);
    prevProgram = firstProgram;
    while((currentProg = strtok(NULL,"|")) != NULL){
        currentProgram = programify(currentProg);
        prevProgram->next = currentProgram;
        prevProgram = currentProgram;
    }
    return firstProgram;
}

job* getJob(pid_t pid){
    job* currentptr = head;
    while(currentptr){
        if(currentptr->pid == pid)
            return currentptr;
    }
    return NULL;
}

job* getHead(){
    return head;
}

void setJob(pid_t pid, pid_t pgid, char* name){
    if(!currentJob){
        currentJob = malloc(sizeof(job));
    }
    job* job = currentJob;
    job->pid = pid;
    job->pgid = pgid;
    job->name = name;
}

void removeJob(pid_t pid){
    job* prevptr = NULL;
    job* currentptr = head;
    while(currentptr){
        if(currentptr->pid == pid){
            if(!prevptr){
                head = currentptr->next;
            }
            else{
                prevptr->next = currentptr->next;
            }
            break;
        }
        prevptr = currentptr;
        currentptr = currentptr->next;
    }
}

void addJob(job* nJob){
    job* currJob = malloc(sizeof(job));
    currJob->pid = nJob->pid;
    currJob->pgid = nJob->pgid;
    currJob->name = nJob->name;
    if(!head){
        jobNum = 1;
        currJob->next = NULL;
        currJob->jobNum = jobNum;
        head = currJob;
    }
    else{
        currJob->next = head;
        currJob->jobNum = jobNum;
        head = currJob;
    }
    jobNum++;
}
void sigint_handler(){
    kill(currentJob->pid,SIGINT);
}

void sigtstp_handler(){
    puts("BLOOBIDYBLOO");
    int status;
    waitpid(-1,&status,WNOHANG);

    addJob(currentJob);
    setpgid(currentJob->pid,currentJob->pid);
    kill(currentJob->pid,SIGTSTP);
}

void sigchld_handler(int sig){
    int status;
    int x = WIFEXITED(status);
    int y = WEXITSTATUS(status);
    if(x != 0){
        puts("hi");
    }
    else{
            puts("bye");
            sigtstp_handler();
    }
}


