#ifndef SHELL_H_
#define SHELL_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <signal.h>
#include <errno.h>
#include <assert.h>
 
#include <unistd.h>
#include <sched.h>

#define LINE_SIZE 512
#define ARG_SIZE  64

typedef struct _Command Command;


// error handlers
#define HANDLE_NULL( a ) {if (a == NULL) { \
                            printf( "Host memory failed in %s at line %d\n", \
                                    __FILE__, __LINE__ ); \
                            exit( EXIT_FAILURE );}}

#define HANDLE_EOF( a ) {if (a == EOF) { \
                            printf( "File reading failed in %s at line %d\n", \
                                    __FILE__, __LINE__ ); \
                            exit( EXIT_FAILURE );}}



// declarate functions
void interactive();

void batch(char *batchfile);

int find_commandNum(char *line);

char *remove_whitespace (char *line);

int split_old(const char *delim, char *src, char **dst);



// void execute(int size,char *commands[LINE_SIZE][ARG_SIZE], int *flag);

// void child(char *command[LINE_SIZE][ARG_SIZE], int id);

void kill_children(pid_t *pid, int start, int size);

int get_command_copy (char* copy, char *command[ARG_SIZE], int id);

int has_pipe(char *command, int size);



int split_commands(const char *delim, char *src, Command *commands);
int split_args(const char *delim, Command *commands, int id);
void free_commands(Command *commands);
void child(Command cmd, int id);
void execute(int size,Command *commands);

#endif