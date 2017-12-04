#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "debug.h"
#include "sfish.h"
#include <signal.h>
#include <errno.h>

int main(int argc, char *argv[], char* envp[]) {
    char* input;
    bool exited = false;
    char cwd[1024];
    char prevwd[1024];
    pid_t pgid;
    int stdin = 0;
    int stdout = 1;
    int stderr = 2;
    int mypgid = getpgid(getpid());
    setParentJob(getpid(),mypgid,*argv);
    unsetenv("OLDPWD");
    if (getcwd(cwd, sizeof(cwd)) == NULL)
       perror("getcwd() error");

    char* prompt = makePrompt(cwd, getenv("HOME"));

    if(!isatty(STDIN_FILENO)) {
        // If your shell is reading from a piped file
        // Don't have readline write anything to that file.
        // Such as the prompt or "user input"
        if((rl_outstream = fopen("/dev/null", "w")) == NULL){
            perror("Failed trying to open DEVNULL");
            exit(EXIT_FAILURE);
        }
    }

    do {
        signal(SIGINT,SIG_IGN);
        signal(SIGTSTP,SIG_IGN);
        signal(SIGTTIN,&sigttin_handler);
        input = readline(prompt);

        //write(1, "\e[s", strlen("\e[s"));
        //write(1, "\e[20;10H", strlen("\e[20;10H"));
        //write(1, "SomeText", strlen("SomeText"));
        //write(1, "\e[u", strlen("\e[u"));
        // If EOF is read (aka ^D) readline returns NULL
        if(input == NULL) {
            puts("");
            continue;
        }
        else{
            char* test = input;
            bool isWS = true;
            while(*test != '\0'){
                if(*test != ' ' && *test != '\n' && *test != '\t'){
                    isWS = false;
                }
                test++;
            }
            if(isWS){
                continue;
            }
        }
        char* input2 = strdup(input);
        //token* currentT = makeTokens(input2);
        //token* firstT = currentT;
        pid_t pid;
        int i  = 0;
        int /*child_status, */ ifd = 0, ofd = 1, pfd[2];
        FILE* file;
        //program* currentProgram = parseTokens(firstT);
        program* currentProgram = makePrograms(input2);
        switch(currentProgram->programType){
            case BAD:
            if(currentProgram->itype == LRED1)
                printf(SYNTAX_ERROR, "Cannot have two '<'");
            else if(currentProgram->itype == RRED1)
                printf(SYNTAX_ERROR, "Cannot have two '>'");
            else if(currentProgram->itype == NULLTYPE)
                printf(SYNTAX_ERROR, "No argument after redirection or pipe");
            break;
            case HELP:
                if(currentProgram->outfile != NULL){
                    file = fopen(currentProgram->outfile, "w");
                    ofd = fileno(file);
                    int stdout_copy = dup(1);
                    dup2(ofd,1);
                    printf(BUILTIN_HELP);
                    dup2(stdout_copy,1);
                }
                else{
                    printf(BUILTIN_HELP);
                }
                break;
            case EXIT: exit(3); break;
            case CD:;
                char** args = currentProgram->args;
                if(*(args+1) == NULL){
                    //Do case of home directory
                    strcpy(prevwd, getenv("PWD"));
                    setenv("OLDPWD",getenv("PWD"),1);
                    chdir(getenv("HOME"));
                    getcwd(cwd,sizeof(cwd));
                    setenv("PWD",getenv("HOME"),1);
                    prompt = makePrompt(cwd, getenv("HOME"));
                }
                else if(strcmp(*(args+1), "-") == 0){
                    //last directory was in
                    if(getenv("OLDPWD") == NULL){
                        puts("THERE IS NO DIR");
                    }
                    else{
                        char* temp;
                        temp = strdup(getenv("OLDPWD"));
                        setenv("OLDPWD",cwd,1);
                        chdir(temp);
                        setenv("PWD",temp,1);
                        getcwd(cwd,sizeof(cwd));
                        strcpy(prevwd,getenv("OLDPWD"));
                        prompt = makePrompt(cwd, getenv("HOME"));
                    }
                }
                else{
                    if((chdir(*(args+1))) == -1){
                        puts("ERROR: Could not change directory");
                    }
                    else{
                        if(strcmp(*(args+1), ".")){
                            strcpy(prevwd,cwd); //PUT CWD INTO PREVWD SINCE SAME
                            setenv("OLDPWD", cwd, 1);
                            getcwd(cwd, sizeof(cwd));
                            setenv("PWD",cwd, 1);
                        }
                        else if(strcmp(*(args+1), "..")){
                            strcpy(prevwd,cwd);
                            getcwd(cwd,sizeof(cwd));
                        }
                        else{
                            strcpy(prevwd, cwd);
                            setenv("OLDPWD", cwd,1);
                            getcwd(cwd, sizeof(cwd));
                            setenv("PWD", cwd, 1);
                        }
                        prompt = makePrompt(cwd, getenv("HOME"));
                    }
                }
            break;
            case PWD:
                if(currentProgram->outfile != NULL){
                    file = fopen(currentProgram->outfile, "w");
                    ofd = fileno(file);
                    int stdout_copy = dup(1);
                    dup2(ofd,1);
                    printf("Current Working Directory: %s\n",getcwd(cwd,sizeof(cwd)));
                    dup2(stdout_copy,1);
                }
                else{
                    printf("Current Working Directory: %s\n",getcwd(cwd,sizeof(cwd)));
                }
                break;
            break;
            case JOBS:;
                job* currJob = getHead();
                while(currJob){
                    printf(JOBS_LIST_ITEM,currJob->jobNum,currJob->name);
                    currJob = currJob->next;
                }
                break;
            case FG:;
                args = currentProgram->args;
                char* jidString = *(args+1);
                if(!jidString){
                    printf(SYNTAX_ERROR,"Must have a JID");
                    break;
                }
                if(*jidString != '%'){
                    printf(SYNTAX_ERROR,"JID must be preceded by '%'.");
                    break;
                }
                int jid = strtol(jidString+1,NULL,10);
                if(jid == 0){
                    printf(SYNTAX_ERROR, "JID must be a valid number.");
                    break;
                }
                currJob = getHead();
                while(currJob){
                    if(currJob->jobNum == jid){
                        break;
                    }
                    currJob=currJob->next;
                }
                setJob(currJob->pid,currJob->pgid,currJob->name);
                kill(currJob->pid,SIGCONT);
                tcsetpgrp(STDOUT_FILENO,currJob->pgid);
                removeJob(currJob->pid);
                break;
            case KILL:;
                args = currentProgram->args;
                char* idString = *(args+1);
                int id;
                if(*idString != '%'){
                    //is a PID
                    id = strtol(idString,NULL,10);
                    if(id ==0){
                        printf(SYNTAX_ERROR, "Invalid PID.");
                        break;
                    }
                }
                else{
                    //is a JID
                    id = strtol(idString+1,NULL,10);
                    if(id ==0){
                        printf(SYNTAX_ERROR, "Invalid JID.");
                        break;
                    }
                    currJob = getHead();
                    while(currJob){
                        if(currJob->jobNum == id){
                            id = currJob->pid;
                            break;
                        }
                    }
                }
                kill(id,SIGKILL);
                removeJob(id);
                break;
            default:;

///////////////////////PROGRAM IS RUN///////////////////////

            //RUNS A PROGRAM
            program* test = currentProgram;
            int numPrograms = 0;
            //Loop to find number of programs;
            while(test!= NULL){
                numPrograms++;
                test = test->next;
            }

            //Loop for piping

            while(currentProgram != NULL){
                //ERROR TESTING
                if(strcmp(currentProgram->name, ">") == 0){
                    printf(SYNTAX_ERROR,"Cannot use > as program argument!");
                    break;
                }
                else if(strcmp(currentProgram->name, "<") == 0){
                    printf(SYNTAX_ERROR,"Cannot use > as program argument!");
                    break;
                }
                else if(strcmp(currentProgram->name, "|") == 0){
                    printf(SYNTAX_ERROR,"Cannot use > as program argument!");
                    break;
                }
                if(currentProgram->programType != PROGRAM){
                    printf(SYNTAX_ERROR,"Cannot use this program here");
                    break;
                }

                //make a pipe
                pipe(pfd);
                sigset_t mask, prev;
                signal(SIGCHLD, &sigchld_handler);
                signal(SIGINT, &sigint_handler);
                signal(SIGTSTP, &sigtstp_handler);
                //signal(SIGTTIN,SIG_IGN);
                sigemptyset(&mask);
                sigaddset(&mask,SIGCHLD);
                sigaddset(&mask,SIGTSTP);
                sigaddset(&mask,SIGINT);
                sigaddset(&mask,SIGCONT);
                sigprocmask(SIG_BLOCK,&mask,&prev);
                if((pid = fork()) == 0){
        ///////////CHILD//////////////
                   /////
                    dup2(ifd,0);
                    if(currentProgram->next != NULL){
                        //THERE ARE MORE PROGRAMS
                        if(currentProgram->itype != NONE){
                            if(i > 0){
                                printf(SYNTAX_ERROR, "Cannot have input here!");
                                exit(3);
                                break;
                            }
                            if((ifd = open(currentProgram->infile, O_RDONLY)) == -1){
                                printf(SYNTAX_ERROR, "No such input file or directory!");
                                exit(3);
                            }
                            dup2(ifd,0);
                        }
                        if(currentProgram->otype != NONE){
                            printf(SYNTAX_ERROR, "Cannot have output here!");
                            exit(3);
                            break;
                        }
                    }
                    else{
                        //This is the last program, or only program.
                        if(currentProgram->infile != NULL){
                            if(numPrograms > 1){
                                //There is more than one program, so this must be the last program
                                printf(SYNTAX_ERROR, "Cannot have input here!");
                                exit(3);
                                break;
                            }
                            if((ifd = open(currentProgram->infile, O_RDONLY)) == -1){
                                printf(EXEC_ERROR, "No such input file or directory!");
                                exit(3);
                            }
                            dup2(ifd,0);
                        }
                        if(currentProgram->outfile != NULL){
                            file = fopen(currentProgram->outfile, "w");
                            ofd = fileno(file);
                            dup2(ofd,1);
                        }
                    }
                    if(currentProgram->next != NULL){
                        dup2(pfd[1],1);
                    }
                    close(pfd[0]);
                    sigprocmask(SIG_SETMASK,&prev,NULL);
                    if(execvp(currentProgram->name,currentProgram->args) == -1){
                        if(errno == 2){
                            printf(EXEC_NOT_FOUND,currentProgram->name);
                        }
                        else{
                            printf(EXEC_ERROR, "Could not run exec");
                        }
                    }
                    exit(3);
                }

                else{

            ////////PARENT///////
                    //////
                    //install signal handler for SIGCHLD

                    //SET THE CURRENT JOB
                    if(i == 0){ //first job
                        pgid = pid;
                        setJob(pid,pid,currentProgram->name);
                        setpgid(pid,pid);
                        tcsetpgrp(STDOUT_FILENO,pgid);
                    }
                    else{
                        setJob(pid,pgid,currentProgram->name);
                        setpgid(pid,pgid);
                    }
                    sigsuspend(&prev);
                    sigprocmask(SIG_SETMASK,&prev,NULL);
                    close(pfd[1]);
                    ifd = pfd[0];
                    currentProgram = currentProgram->next;
                }
                i++;
            }
            break;
        }
        // // Currently nothing is implemented
        // printf(EXEC_NOT_FOUND, input);

        // // You should change exit to a "builtin" for your hw.
        // exited = strcmp(input, "exit") == 0;

        // Readline mallocs the space for input. You must free it.
        rl_free(input);
        free(input2);
        signal(SIGTTOU, SIG_IGN);
        tcsetpgrp(STDOUT_FILENO,mypgid);
    } while(!exited);

    return EXIT_SUCCESS;
}
