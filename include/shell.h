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
#define HISTORY_SIZE 20 // New: Define max history size

// Global variables for history management
extern char* history[HISTORY_SIZE];
extern int history_count; // Tracks the total number of commands stored

// Function prototypes
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char* arglist[]);
void add_to_history(char* cmd); // New: Function to add command to history

#endif // SHELL_H
