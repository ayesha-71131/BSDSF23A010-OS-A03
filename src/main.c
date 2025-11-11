#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;
    // Removed: int result; 

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {
        if ((arglist = tokenize(cmdline)) != NULL) {

            // execute(arglist); // Direct call instead of assignment
            execute(arglist); 

            // Memory Cleanup 
            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }
            free(arglist);
        }
        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
