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

int main(int argc, char *argv[], char* envp[]) {
    char* input;
    bool exited = false;
    char cwd[1024];
    char prevwd[1024];
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

        input = readline(prompt);

        write(1, "\e[s", strlen("\e[s"));
        write(1, "\e[20;10H", strlen("\e[20;10H"));
        //write(1, "SomeText", strlen("SomeText"));
        write(1, "\e[u", strlen("\e[u"));
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
        token* currentT = makeTokens(input2);
        token* firstT = currentT;
        pid_t pid;
        int i = 0;
        int child_status, ifd = 0, ofd = 1, pfd[2];
        FILE* file;
        program* currentProgram = parseTokens(firstT);
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

            default:;

            //RUNS A PROGRAM
            program* test = currentProgram;
            int numPrograms = 0;
            while(test!= NULL){
                numPrograms++;
                test = test->next;
            }
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
                pipe(pfd);

                if((pid = fork()) == 0){
                //CHILD
                    dup2(ifd,0);
                    if(currentProgram->next != NULL){
                        //THERE ARE MORE PROGRAMS
                        if(currentProgram->itype != NONE){
                            if(i > 0){
                                printf(SYNTAX_ERROR, "Cannot have input here!");
                                break;
                            }
                            if((ifd = open(currentProgram->infile, O_RDONLY)) == -1){
                                printf(SYNTAX_ERROR, "No such input file or directory!");
                                exit(0);
                            }
                            dup2(ifd,0);
                        }
                        if(currentProgram->otype != NONE){
                            printf(SYNTAX_ERROR, "Cannot have output here!");
                            break;
                        }
                    }
                    else{
                        //This is the last program, or only program.
                        if(currentProgram->infile != NULL){
                            if(numPrograms > 1){
                                //There is more than one program, so this must be the last program
                                printf(SYNTAX_ERROR, "Cannot have input here!");
                                break;
                            }
                            if((ifd = open(currentProgram->infile, O_RDONLY)) == -1){
                                printf(EXEC_ERROR, "No such input file or directory!");
                                exit(0);
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
                    if(execvp(currentProgram->name,currentProgram->args) == -1){
                        printf(EXEC_ERROR, "Invalid program");
                    }
                    exit(0);
                }

                else{
                 //PARENT
                    wait(&child_status);
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

    } while(!exited);

    debug("%s", "user entered 'exit'");

    return EXIT_SUCCESS;
}
