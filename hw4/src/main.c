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
    if (getcwd(cwd, sizeof(cwd)) != NULL)
       fprintf(stdout, "Current working dir: %s\n Home dir: %s\n ", cwd, getenv("HOME"));
    else
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
        char* input2 = strdup(input);
        token* currentT = makeTokens(input2);
        token* firstT = currentT;
        // for(int i = 0; currentT!= NULL; i++){
        //     printf("Token %i is: %s with type: %i\n", i, currentT->contents, currentT->type);
        //     currentT = currentT->next;
        // }
        if(input2 == NULL) {
            continue;
        }

        program* currentProgram = parseTokens(firstT);
        while(currentProgram != NULL){
            switch(currentProgram->programType){
                case HELP: BUILTIN_HELP break;
                case EXIT: exited = true; break;
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
                printf("Current Working Directory: %s\n",getcwd(cwd,sizeof(cwd)));
                break;
                default:;
                //RUNS A PROGRAM
                pid_t pid;
                pid = fork();
                int child_status;
                int ifd;
                int ofd;
                FILE* file;
                if(pid == 0){ //CHILD
                    if(currentProgram->infile != NULL){
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
                    execvp(currentProgram->name,currentProgram->args);
                    exit(0);
                }

                else{
                    wait(&child_status);
                }
                break;
            }
            currentProgram = currentProgram->next;
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
