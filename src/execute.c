#include "shell.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h> // Ensure this is included for getenv()

/**
 * Executes a command given its tokenized argument list.
 * Checks for built-in commands (like 'exit', 'cd', 'jobs', 'help') before forking
 * to execute external programs.
 */
int execute(char* arglist[]) {
    // Check for built-in commands (Feature 2, Task 2)
    
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

    // 3. Built-in 'jobs' (Placeholder/Simple implementation)
    if (strcmp(arglist[0], "jobs") == 0) {
        printf("jobs: No background jobs are currently running (job control not yet fully implemented).\n");
        return 0; // Handled built-in
    }

    // 4. Built-in 'help'
    if (strcmp(arglist[0], "help") == 0) {
        printf("\n--- My Simple Shell Help ---\n");
        printf("Available built-in commands:\n");
        printf("  exit         : Terminate the shell.\n");
        printf("  cd [DIR]     : Change the current directory (defaults to HOME).\n");
        printf("  help         : Display this help information.\n");
        printf("  jobs         : Display background jobs (currently a placeholder).\n");
        printf("\nAll other commands are executed externally (e.g., 'ls', 'pwd').\n");
        printf("------------------------------\n");
        return 0; // Handled built-in
    }

    // --- External Command Execution (Original Logic) ---
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
