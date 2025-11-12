#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

// Feature 4: List of built-in commands for completion 
const char* builtins[] = {
    "exit",
    "cd",
    "history",
    "ls", // Adding common external commands helps completion
    "pwd",
    "whoami",
    NULL // Sentinel value to mark the end of the array
};

// The read_cmd function using readline (Feature 4, Task 1)
char* read_cmd(char* prompt) {
    char* cmdline;
    // readline handles memory allocation internally
    cmdline = readline(prompt);

    if (cmdline == NULL) {
        // EOF (Ctrl+D) reached
        return NULL;
    }

    if (cmdline[0] == '\0') {
        // Empty line, free and return a flag (empty string) to main
        free(cmdline); 
        return strdup(""); 
    }

    // Add non-empty command to readline's history list
    // Note: We also add it to our custom history array in main.c
    // add_history(cmdline); // Commented out, now done in main.c

    return cmdline;
}


// --- Feature 5: MODIFIED TOKENIZER for Redirection and Pipes ---
// Now handles spaces, tabs, and treats '<', '>', '|' as separate tokens.
char** tokenize(char* cmdline) {
    // Edge case: empty command line (should be handled in read_cmd, but defensive check)
    if (cmdline == NULL || cmdline[0] == '\0') {
        return NULL;
    }

    // Allocate array of char pointers (MAXARGS + 1 for the NULL sentinel)
    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    if (arglist == NULL) {
        perror("myshell: malloc arglist failed");
        return NULL;
    }
    
    // Allocate space for each token string
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        if (arglist[i] == NULL) {
             perror("myshell: malloc arglist[i] failed");
             // Cleanup already allocated memory
             for(int j = 0; j < i; j++) free(arglist[j]);
             free(arglist);
             return NULL;
        }
        arglist[i][0] = '\0'; // Initialize string
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    // Loop until end of string or max arguments reached
    while (*cp != '\0' && argnum < MAXARGS) {
        // 1. Skip leading whitespace
        while (*cp != '\0' && (*cp == ' ' || *cp == '\t')) {
            cp++;
        }
        
        // If end of string after skipping whitespace, break
        if (*cp == '\0') {
            break;
        }
        
        // 2. --- Handle Special Characters: <, >, >>, | ---
        if (*cp == '<' || *cp == '|' || *cp == '>') {
            start = cp;
            len = 1;

            // Check for '>>' append redirection
            if (*cp == '>' && *(cp + 1) == '>') {
                len = 2;
                cp++;
            }
            
            cp++; // Move past the special character(s)
        } 
        
        // 3. --- Handle Regular Arguments (non-whitespace, non-special) ---
        else {
            start = cp;
            len = 1;
            
            // Loop until end of string, whitespace, or a special character is found
            while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t') && !(*cp == '<' || *cp == '>' || *cp == '|')) {
                len++;
            }
        }
        
        // Copy the token (argument or special character)
        if (len > 0) {
            strncpy(arglist[argnum], start, len);
            arglist[argnum][len] = '\0'; // Null-terminate the token
            argnum++;
        }
    }


    if (argnum == 0) { // No arguments were parsed (e.g., input was only spaces)
        // Clean up all allocated memory
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }


    arglist[argnum] = NULL; // NULL sentinel for the execute function
    return arglist;
}

// --- Feature 4, Task 2: Completion Functions ---

/**
 * Generates a list of possible completions for a given command prefix.
 * This is the function called by readline's completion mechanism.
 */
char* command_generator(const char *text, int state) {
    static int list_index, len;
    char *name;

    if (!state) {
        // New completion attempt: initialize variables
        list_index = 0;
        len = strlen(text);
    }

    // Loop through the list of known built-ins
    while ((name = (char*)builtins[list_index++])) {
        if (strncmp(name, text, len) == 0) {
            // Found a match
            return strdup(name);
        }
    }

    // No more matches in built-ins
    return (char*)NULL;
}

/**
 * Main completion function hook for readline.
 * It is responsible for setting up the completion lists.
 */
char** myshell_completion(const char *text, int start, int end) {
    // start and end mark the token being completed in the command line
    char** matches = (char**)NULL;

    // We only want to attempt completion for the first word (the command name)
    // start == 0 means the cursor is at the beginning of the command line
    if (start == 0) {
        // Use the custom command_generator to find matches in our built-ins list
        matches = rl_completion_matches(text, command_generator);
    } 
    // You could add logic here for filename completion on other tokens
    // else {
    //     matches = rl_filename_completion_function(text, start, end);
    // }

    return matches;
}
