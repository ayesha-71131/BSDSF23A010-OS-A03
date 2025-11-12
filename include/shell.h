#ifndef SHELL_H
#define SHELL_H

#include <readline/readline.h>
#include <readline/history.h>

// --- Shell Constants ---
#define PROMPT "myshell> "
#define MAXARGS 32      // Max number of arguments + command name
#define ARGLEN 256      // Max length of a single argument
#define HISTORY_SIZE 10 // Max number of history entries

// --- Global History Variables (Extern Declaration) ---
// Declared in main.c, used in shell.c and execute.c
extern char* history[];
extern int history_count;

// --- Function Prototypes (shell.c) ---
char* read_cmd(char* prompt);
char** tokenize(char* cmdline);

// Readline Completion Functions (shell.c)
char* command_generator(const char *text, int state);
char** myshell_completion(const char *text, int start, int end);

// --- Function Prototypes (main.c) ---
void add_to_history(char* cmd);
int handle_re_execute(char** original_cmdline);
void setup_signals();

// --- Function Prototypes (execute.c) ---
int execute(char* arglist[]);
int execute_builtin(char* arglist[]);

#endif // SHELL_H
