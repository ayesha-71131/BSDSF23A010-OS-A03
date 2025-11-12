#ifndef SHELL_H
#define SHELL_H

#include <readline/readline.h>
#include <readline/history.h>

// --- Shell Constants ---
#define PROMPT "myshell> "
#define MAXARGS 32      // Max number of arguments + command name
#define ARGLEN 256      // Max length of a single argument
#define HISTORY_SIZE 10 // Max number of history entries
#define MAX_JOBS 32     // Feature 6: Max background jobs

// --- Global History Variables (Extern Declaration) ---
// Declared in main.c, used in shell.c and execute.c
extern char* history[];
extern int history_count;

// --- Feature 6: Job Management Structures ---
typedef struct {
    pid_t pid;      // Process ID
    char* cmd;      // Command string
    int job_id;     // Job ID number
} job_t;

extern job_t jobs[MAX_JOBS];
extern int job_count;
extern int next_job_id;

// --- Function Prototypes (shell.c) ---
char* read_cmd(char* prompt);
char** tokenize(char* cmdline);
char** split_commands(char* cmdline);  // FEATURE 6: Command chaining

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
int execute_single_command(char* arglist[], int is_piped, int is_chained, char* original_cmd);
void init_jobs(void);
void reap_zombies(void);
void add_job(pid_t pid, char* cmd);
void remove_job(pid_t pid);

#endif // SHELL_H
