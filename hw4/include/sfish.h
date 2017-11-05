#ifndef SFISH_H
#define SFISH_H
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Format Strings */
#define EXEC_NOT_FOUND "sfish: %s: command not found\n"
#define JOBS_LIST_ITEM "[%d] %s\n"
#define STRFTIME_RPRMT "%a %b %e, %I:%M%p"
#define BUILTIN_ERROR  "sfish builtin error: %s\n"
#define SYNTAX_ERROR   "sfish syntax error: %s\n"
#define EXEC_ERROR     "sfish exec error: %s\n"
#define BUILTIN_HELP "help: prints a list of commands \n\nexit: exit shell\n\ncd: change current working directory\n\tcd <->: should change to the last directory the user was in\n\n\tcd <no args>: goes to user's home directory\n\npwd: print absolute path to current working directory\n"

typedef enum tokenType{PIPE = 1, LRED = 77, RRED = 7, OUT = 3, IN = 4, PROG = 0, ARG = 2} TokenType;
typedef struct token token;

struct token {
    TokenType type;
    char* contents;
    token* next;
};

typedef enum ioType{RRED1, LRED1, RRED2, LRED2, NONE, NULLTYPE} IoType;
typedef enum progType{HELP, EXIT, CD, PWD, PROGRAM, BAD, JOBS, FG, KILL} ProgType;
typedef struct program program;
struct program {
    ProgType programType;
    char* name;
    char* infile;
    char* outfile;
    IoType itype;
    IoType otype;
    program* next;
    char** args;
};
typedef struct job job;
struct job{
    char* name;
    pid_t pid;
    pid_t pgid;
    int jobNum;
    job* next;
};

char* cwd;

char* makePrompt(char* cwd, char* homedir);
program* programify(char* input);
program* makePrograms(char* input);
void setParentJob();
void sigcont_handler();
void sigchld_handler();
void sigint_handler();
void sigtstp_handler();
void setJob(pid_t pid, pid_t pgid, char* name);
void addJob(job* job);
void removeJob(pid_t pid);
job* getHead();

#endif
