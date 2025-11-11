#include "shell.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h> 

/**
 * Executes a command given its tokenized argument list.
 */
int execute(char* arglist[]) {

    // --- Built-in Command Handling ---

    // 1. Built-in 'exit'
    if (strcmp(arglist[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0); 
    }

    // 2. Built-in 'cd'
    if (strcmp(arglist[0], "cd") == 0) {
        char* dir = arglist[1];
        if (dir == NULL) {
            dir = getenv("HOME");
        }
        if (dir == NULL) {
            fprintf(stderr, "cd: Could not determine home directory.\n");
            return 1;
        }
        
        if (chdir(dir) == -1) {
            perror("cd");
        }
        return 0; // Handled built-in
    }

    // 3. Built-in 'jobs' (Placeholder)
    if (strcmp(arglist[0], "jobs") == 0) {
        printf("jobs: No background jobs are currently running (job control not yet fully implemented).\n");
        return 0; // Handled built-in
    }

    // 4. Built-in 'help' (Updated)
    if (strcmp(arglist[0], "help") == 0) {
        printf("\n--- My Simple Shell Help ---\n");
        printf("Available built-in commands:\n");
        printf("  exit         : Terminate the shell.\n");
        printf("  cd [DIR]     : Change the current directory (defaults to HOME).\n");
        printf("  help         : Display this help information.\n");
        printf("  jobs         : Display background jobs (currently a placeholder).\n");
        printf("  history      : Display the command history.\n"); // Added
        printf("\nAll other commands are executed externally (e.g., 'ls', 'pwd').\n");
        printf("------------------------------\n");
        return 0; // Handled built-in
    }

    // 5. Built-in 'history' (New)
    if (strcmp(arglist[0], "history") == 0) {
        int start_index = (history_count > HISTORY_SIZE) ? history_count - HISTORY_SIZE : 0;
        int current_idx = history_count % HISTORY_SIZE;
        
        for (int i = start_index; i < history_count; i++) {
            int h_index = i % HISTORY_SIZE;
            if (history[h_index] != NULL) {
                // The line number is (i + 1)
                printf("  %d  %s\n", i + 1, history[h_index]);
            }
        }
        return 0; // Handled built-in
    }

    // --- External Command Execution ---
    int status;
    int cpid = fork();
    
    switch (cpid) {
        case -1:
            perror("fork failed");
            exit(1);
        case 0: // Child process
            execvp(arglist[0], arglist);
            perror("Command not found"); // This line runs only if execvp fails
            exit(1);
        default: // Parent process
            waitpid(cpid, &status, 0);
            return 0;
    }
}
