#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <strings.h> // For bzero

#define MAX_LEN 256
#define MAXARGS 16
#define ARGLEN 128
#define PROMPT "myshell> "

// Function prototypes
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char* arglist[]);

#endif // SHELL_H
