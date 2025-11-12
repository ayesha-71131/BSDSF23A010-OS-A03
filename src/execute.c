#include "shell.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h> 
#include <fcntl.h> // For open() flags (O_RDONLY, O_WRONLY, etc.)
#include <errno.h>
#include <sys/wait.h> // For waitpid

#define MAX_COMMANDS_IN_PIPE 2 // Limiting to one pipe (two commands) for simplicity

// Helper structure to hold command arguments and file paths
typedef struct {
    char* args[MAXARGS]; // Array for command arguments (before redirection)
    char* input_file;    // Filename for < redirection
    char* output_file;   // Filename for > redirection
    char* append_file;   // Filename for >> redirection
} command_info_t;

// --- Built-in Commands Implementation (Feature 2 & 3) ---

/**
 * Executes a built-in command if found.
 * Returns 1 if a built-in was executed, 0 otherwise.
 */
int execute_builtin(char* arglist[]) {
    if (arglist[0] == NULL) {
        return 0;
    }

    // exit
    if (strcmp(arglist[0], "exit") == 0) {
        exit(0);
    } 
    // cd 
    else if (strcmp(arglist[0], "cd") == 0) {
        // Feature 2: Handle "cd" with no argument (cd ~) or with an argument
        char *path = (arglist[1] != NULL) ? arglist[1] : getenv("HOME");
        
        if (path == NULL) {
            path = "/"; // Fallback
        }

        if (chdir(path) != 0) {
            perror("myshell: cd failed");
        }
        return 1;
    }
    // history
    else if (strcmp(arglist[0], "history") == 0) {
        // Feature 3: Print history 
        extern char* history[];
        extern int history_count;
        int start_idx = (history_count > HISTORY_SIZE) ? history_count - HISTORY_SIZE : 0;
        
        for (int i = start_idx; i < history_count; i++) {
            int current_idx = i % HISTORY_SIZE;
            if (history[current_idx] != NULL) {
                printf("%d  %s\n", i + 1, history[current_idx]);
            }
        }
        return 1;
    }
    // === ADDED: help command (Feature 2) ===
    else if (strcmp(arglist[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  cd [directory]    - Change directory\n");
        printf("  exit              - Exit the shell\n");
        printf("  history           - Show command history\n");
        printf("  help              - Show this help message\n");
        printf("  jobs              - Job control (not implemented)\n");
        return 1;
    }
    // === ADDED: jobs command (Feature 2) ===
    else if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    return 0;
}


// --- Helper function to find the pipe symbol (Feature 5) ---
int find_pipe(char* arglist[]) {
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "|") == 0) {
            return i; // Returns the index of the '|'
        }
    }
    return -1; // No pipe found
}


// --- I/O Redirection & Single Command Execution (Feature 5) ---

/**
 * Parses the command line for redirection symbols and populates command_info_t.
 * The command arguments array is null-terminated at the redirection point.
 * Returns the index in arglist where the command ends (before redirection/pipe).
 */
int parse_command_for_redirection(char* arglist[], command_info_t* cmd_info) {
    int cmd_end_idx = 0;
    for (int i = 0; arglist[i] != NULL; i++) {
        
        // Redirection symbols must be handled by the shell, not passed to execvp
        if (strcmp(arglist[i], "<") == 0) {
            if (arglist[i + 1] != NULL) {
                cmd_info->input_file = arglist[i + 1];
                i++; // Skip the filename
            } else {
                fprintf(stderr, "myshell: syntax error: expected argument after '<'\n");
                return -1; // Error
            }
        } 
        else if (strcmp(arglist[i], ">") == 0) {
            if (arglist[i + 1] != NULL) {
                cmd_info->output_file = arglist[i + 1];
                i++; // Skip the filename
            } else {
                fprintf(stderr, "myshell: syntax error: expected argument after '>'\n");
                return -1; // Error
            }
        } 
        else if (strcmp(arglist[i], ">>") == 0) {
            if (arglist[i + 1] != NULL) {
                cmd_info->append_file = arglist[i + 1];
                i++; // Skip the filename
            } else {
                fprintf(stderr, "myshell: syntax error: expected argument after '>>'\n");
                return -1; // Error
            }
        } 
        else {
            // This is a regular command argument
            cmd_info->args[cmd_end_idx++] = arglist[i];
            
            // Check for pipe, which is handled in execute() main function
            if (strcmp(arglist[i], "|") == 0) {
                cmd_end_idx--; // Do not include '|' in args
                break;
            }
        }
    }
    
    // Null-terminate the command arguments array
    cmd_info->args[cmd_end_idx] = NULL; 
    return cmd_end_idx;
}

/**
 * Handles I/O redirection using dup2() and open().
 * Returns 0 on success, 1 on error.
 */
int process_redirection(command_info_t* cmd_info) {
    int fd;
    
    // 1. Input Redirection (<)
    if (cmd_info->input_file) {
        fd = open(cmd_info->input_file, O_RDONLY);
        if (fd == -1) {
            perror("myshell: failed to open input file");
            return 1;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    
    // 2. Output Redirection (>)
    if (cmd_info->output_file) {
        // O_CREAT: Create file if it does not exist
        // O_TRUNC: Truncate to zero length if file exists
        // O_WRONLY: Write only
        fd = open(cmd_info->output_file, O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if (fd == -1) {
            perror("myshell: failed to open output file");
            return 1;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    
    // 3. Append Redirection (>>)
    if (cmd_info->append_file) {
        // O_CREAT: Create file if it does not exist
        // O_APPEND: Append to the end of file
        // O_WRONLY: Write only
        fd = open(cmd_info->append_file, O_CREAT | O_APPEND | O_WRONLY, 0644);
        if (fd == -1) {
            perror("myshell: failed to open append file");
            return 1;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    
    return 0; // Success
}

/**
 * Executes a single command (potentially with I/O redirection).
 * This function is used by both the main execute() and the pipe handler.
 * is_piped: 1 if this execution is part of a pipe, 0 otherwise.
 */
int execute_single_command(char* arglist[], int is_piped) {
    command_info_t cmd_info = {0};
    pid_t pid;
    int status;
    
    // Handle the case where the command list is empty (e.g., just redirection)
    if (arglist[0] == NULL) {
        return 0;
    }
    
    // 1. Parse Redirection
    if (parse_command_for_redirection(arglist, &cmd_info) == -1) {
        return 1; // Error during parsing
    }

    // Check for built-ins *before* fork/exec cycle, but *after* argument parsing.
    // NOTE: Built-ins should not be part of a pipe (e.g., 'cd | grep').
    // If a built-in is found, we execute it in the parent shell process.
    if (!is_piped && execute_builtin(cmd_info.args)) {
        return 0; // Built-in successfully executed
    }
    
    // 2. Fork and Execute External Command
    pid = fork();
    
    if (pid == -1) { // Fork failed
        perror("myshell: fork failed");
        return 1;
    }
    
    if (pid == 0) { // Child process
        
        // Reset SIGINT handler to default for the child process. 
        // This ensures the command is killable by Ctrl+C.
        signal(SIGINT, SIG_DFL);
        
        // 2a. Process Redirection (dup2 calls will modify child's file descriptors)
        if (process_redirection(&cmd_info) == 1) {
            exit(1); // Exit child on redirection error
        }
        
        // 2b. Execute the command
        execvp(cmd_info.args[0], cmd_info.args);
        
        // execvp only returns on error
        fprintf(stderr, "myshell: %s: command not found\n", cmd_info.args[0]);
        exit(1); // Exit child on execvp failure
    }
    
    // Parent process: Wait for the child to finish
    if (waitpid(pid, &status, 0) == -1) {
        // This waitpid call is crucial for sequential execution
        perror("myshell: waitpid failed");
        return 1;
    }
    
    return WEXITSTATUS(status);
}

/**
 * Handles the two-command pipe case: cmd_left | cmd_right. (Feature 5)
 */
int execute_pipe(char* arglist[], int pipe_index) {
    int pfd[2]; // Pipe file descriptors: pfd[0] is read, pfd[1] is write
    pid_t cpid1, cpid2;
    int status;

    // 1. Create the pipe
    if (pipe(pfd) == -1) {
        perror("myshell: pipe failed");
        return 1;
    }

    // Split the arglist into two commands
    char** cmd_left = arglist;
    char** cmd_right = arglist + pipe_index + 1;
    
    // Null-terminate the left command list (it's part of the arglist array)
    arglist[pipe_index] = NULL; 

    // --- Fork 1: Command 1 (Writer to pipe) ---
    cpid1 = fork();
    if (cpid1 == -1) {
        perror("myshell: fork failed for pipe command 1");
        return 1;
    }

    if (cpid1 == 0) { // Child 1: Left command
        // Reset SIGINT handler to default for the child process
        signal(SIGINT, SIG_DFL);
        
        close(pfd[0]); // Close read end of pipe (not needed by writer)
        
        // Redirect stdout to pipe's write end
        dup2(pfd[1], STDOUT_FILENO); 
        close(pfd[1]); // Close pipe's write end (dup2 made a copy)
        
        // Execute the left command (handles its own I/O redirection if any)
        execute_single_command(cmd_left, 1);
        exit(1); // execute_single_command handles error exit itself, but defensive exit(1) on function return
    } 

    // --- Fork 2: Command 2 (Reader from pipe) ---
    cpid2 = fork();
    if (cpid2 == -1) {
        perror("myshell: fork failed for pipe command 2");
        // Parent must wait for child 1 even if child 2 fails to fork
        waitpid(cpid1, &status, 0); 
        return 1;
    }

    if (cpid2 == 0) { // Child 2: Right command
        // Reset SIGINT handler to default for the child process
        signal(SIGINT, SIG_DFL);
        
        close(pfd[1]); // Close write end of pipe (not needed by reader)
        
        // Redirect stdin to pipe's read end
        dup2(pfd[0], STDIN_FILENO); 
        close(pfd[0]); // Close pipe's read end
        
        // Execute the right command (handles its own I/O redirection if any)
        execute_single_command(cmd_right, 1);
        exit(1); // defensive exit(1)
    }

    // Parent process: Close the pipe FDs and wait for both children
    close(pfd[0]);
    close(pfd[1]);

    // Wait for both children
    waitpid(cpid1, &status, 0);
    waitpid(cpid2, &status, 0); 
    
    return WEXITSTATUS(status);
}


/**
 * Main entry point for command execution. (Feature 1, 5)
 */
int execute(char* arglist[]) {
    if (arglist == NULL || arglist[0] == NULL) {
        return 0;
    }

    int pipe_index = find_pipe(arglist);

    if (pipe_index != -1) {
        // Handle pipe logic
        return execute_pipe(arglist, pipe_index);
    } else {
        // Handle single command (with possible redirection/built-ins)
        return execute_single_command(arglist, 0);
    }
}
