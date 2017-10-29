#ifndef SFISH_H
#define SFISH_H
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Format Strings */
#define EXEC_NOT_FOUND "sfish: %s: command not found\n"
#define JOBS_LIST_ITEM "[%d] %s\n"
#define STRFTIME_RPRMT "%a %b %e, %I:%M%p"
#define BUILTIN_HELP                                                           \
  do {                                                                         \
    fprintf(stderr,                                                            \
            "\n%s help: prints a list of commands \n"                  \
            "\n"                                                               \
            "exit: exit shell\n" \
            "\n"                                                               \
            "cd: change current working directory"                             \
            "\tcd <->: should change to the last directory the user was in\n"       \
            "\n"              \
            "\tcd <no args>: goes to user's home directory\n"                  \
            "\n"                                         \
            "pwd: print absolute path to current working directory\n",           \
  );} while (0);

typedef enum tokenType{PIPE = 1, LRED = 77, RRED = 7, OUT = 3, IN = 4, PROG = 0, ARG = 2} TokenType;

typedef struct token token;

struct token {
    TokenType type;
    char* contents;
    token* next;
};

typedef enum ioType{RRED,LRED} IoType;
typedef struct program program;
struct program {
    char* name;
    char* args;
    char* infile;
    char* outfile;
    IoType* itype;
    IoType* otype;
}
char* cwd;

char* makePrompt(char* cwd, char* homedir);

token* makeTokens(char* input);

#endif
