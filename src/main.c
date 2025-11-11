#include "shell.h"

// Global history storage variables definition
char* history[HISTORY_SIZE];
int history_count = 0;

void add_to_history(char* cmd) {
    if (cmd == NULL || cmd[0] == '\0') return;

    // Determine the index where the new command will be stored
    int index = history_count % HISTORY_SIZE;

    // If the history slot is already allocated (i.e., we are overwriting an old command), free it first.
    if (history[index] != NULL) {
        free(history[index]);
    }
    
    // Dynamically allocate memory for the command copy and store it
    history[index] = strdup(cmd);

    // Increment the count. It will track the total commands ever entered.
    history_count++;
}

/**
 * Handles the !n re-execution command.
 * If successful, it replaces the content of the original_cmdline pointer.
 * Returns 1 if successful, 0 otherwise.
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

    // Parse the number n that follows '!'
    int n = atoi(cmd + 1); // Skip the '!'
    
    // Command numbers are 1-based, but our history index starts from 0.
    // The command number 'n' corresponds to the (n-1)th total command entered.

    if (n <= 0 || n > history_count || n < history_count - HISTORY_SIZE + 1) {
        fprintf(stderr, "myshell: !%d: event not found\n", n);
        return 0; // Handle errors if n is out of bounds or too old
    }

    // Calculate the index in the circular buffer
    int history_index = (n - 1) % HISTORY_SIZE;
    
    char* command_to_execute = history[history_index];

    if (command_to_execute == NULL) {
         fprintf(stderr, "myshell: !%d: event not found\n", n);
         return 0;
    }

    printf("%s\n", command_to_execute); // Print the retrieved command for verification

    // 1. Free the memory of the current cmdline (which holds the "!n")
    free(*original_cmdline);

    // 2. Duplicate the history command string
    char* new_cmdline = strdup(command_to_execute);
    
    // 3. Update the pointer in main to point to the new command string
    *original_cmdline = new_cmdline; 

    return 1;
}

int main() {
    char* cmdline;
    char** arglist;

    // Initialize history array pointers to NULL
    for (int i = 0; i < HISTORY_SIZE; i++) {
        history[i] = NULL;
    }

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        
        // --- 1. Re-execution Check (New) ---
        // Must happen before adding to history and tokenization
        int is_re_executed = handle_re_execute(&cmdline);
        // The cmdline pointer might be pointing to a new string after this call.
        // ------------------------------------

        // --- 2. History Storage ---
        // Only store the final command (the one that will be executed)
        add_to_history(cmdline);
        
        if ((arglist = tokenize(cmdline)) != NULL) {
            
            execute(arglist); 

            // Memory Cleanup 
            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }
            free(arglist);
        }
        
        // The command line is guaranteed to be a new string if re_execute was successful.
        // If the original command was "!n", 'cmdline' was freed inside 'handle_re_execute'.
        // If 'cmdline' was a normal command, it still needs to be freed here.
        free(cmdline);
    }
    
    // Clean up history memory before exiting
    for (int i = 0; i < HISTORY_SIZE; i++) {
        if (history[i] != NULL) {
            free(history[i]);
        }
    }

    printf("\nShell exited.\n");
    return 0;
}
