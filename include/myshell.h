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

int split(const char *delim, char *src, char **dst);

void execute(int size,char *commands[512][64]);

void child(char *command[512][64], int id);

#endif