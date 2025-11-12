#include "shell.h"
#include <signal.h> // For signal handling
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Feature 3: Global history storage variables DEFINITION
char* history[HISTORY_SIZE];
int history_count = 0;

void add_to_history(char* cmd) {
    if (cmd == NULL || cmd[0] == '\0') return;

    int index = history_count % HISTORY_SIZE;

    // Free the old command at this slot if the history wraps around
    if (history[index] != NULL) {
        free(history[index]);
    }
    
    // Duplicate the string so it's safely stored
    history[index] = strdup(cmd);
    history_count++;

    // Feature 4: Use readline's own history function 
    add_history(cmd); 
}

/**
 * Feature 3: Handles the !n re-execution command.
 */
int handle_re_execute(char** original_cmdline) {
    char* cmd = *original_cmdline;
    if (cmd == NULL || cmd[0] != '!') {
        return 0; // Not a re-execution command
    }

    if (strlen(cmd) == 1) {
        fprintf(stderr, "myshell: !: event not found\n");
        return 0;
    }

    // Convert the number part of the command (e.g., "10" from "!10")
    int n = atoi(cmd + 1); 
    
    // Check if the history index 'n' is valid
    // Must be positive AND within the bounds of what we've stored.
    if (n <= 0 || n > history_count || 
        (history_count > HISTORY_SIZE && n < history_count - HISTORY_SIZE + 1)) {
        
        fprintf(stderr, "myshell: !%d: event not found\n", n);
        return 0; 
    }

    // Calculate the physical index in the circular history array
    int history_index = (n - 1) % HISTORY_SIZE;
    
    char* command_to_execute = history[history_index];

    if (command_to_execute == NULL) {
         fprintf(stderr, "myshell: !%d: event not found\n", n);
         return 0;
    }

    // Print the command being re-executed for user clarity
    printf("%s\n", command_to_execute); 

    // Replace the current command line with the history command
    free(*original_cmdline);
    char* new_cmdline = strdup(command_to_execute);
    *original_cmdline = new_cmdline; 

    return 1; // Successfully handled re-execution
}

// --- Signal Handling for Shell Protection (Feature 1 correction) ---
void setup_signals() {
    // Parent shell IGNORES SIGINT (Ctrl+C). 
    // This ensures Ctrl+C only terminates the foreground child, not the shell.
    signal(SIGINT, SIG_IGN); 
}

int main() {
    char* cmdline;
    char** arglist;

    // 1. Signal Setup for Shell Protection
    setup_signals(); 

    // Feature 3: Initialize history array pointers to NULL
    for (int i = 0; i < HISTORY_SIZE; i++) {
        history[i] = NULL;
    }

    // Feature 4: Setup the Custom Completion Hook 
    rl_attempted_completion_function = myshell_completion;

    // Feature 6: Initialize job system
    init_jobs();

    while ((cmdline = read_cmd(PROMPT)) != NULL) { 
        
        // Handle EOF (Ctrl+D) case where read_cmd returns NULL
        if (cmdline == NULL) {
            break; 
        }

        // Handle empty line case where read_cmd returns ""
        if (cmdline[0] == '\0') {
            free(cmdline);
            continue;
        }

        // Feature 3: Re-execution Check. NOTE: This modifies 'cmdline' if '!' is found.
        handle_re_execute(&cmdline);

        // Feature 3: History Storage
        add_to_history(cmdline);

        // === FEATURE 6: Command Chaining with Semicolon ===
        char** commands = split_commands(cmdline);
        
        if (commands != NULL) {
            // Execute each command sequentially
            for (int i = 0; commands[i] != NULL; i++) {
                char** arglist = tokenize(commands[i]);
                
                if (arglist != NULL) {
                    execute(arglist); 

                    // Memory Cleanup for tokens
                    for (int j = 0; arglist[j] != NULL; j++) {
                        free(arglist[j]);
                    }
                    free(arglist);
                }
                
                // Free the command string
                free(commands[i]);
            }
            free(commands);
        }
        // === END FEATURE 6 ===
        
        // Free the command line memory allocated by readline (or strdup in re-execute)
        free(cmdline);
    }
    
    // Feature 3: Clean up custom history on exit
    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (history[i] != NULL) {
            free(history[i]);
        }
    }

    // Feature 6: Clean up job system
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].cmd != NULL) {
            free(jobs[i].cmd);
        }
    }

    printf("Exiting myshell...\n");
    return 0;
}
