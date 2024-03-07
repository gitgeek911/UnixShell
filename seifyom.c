//Libraries Included
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // POSIX API access (fork, execvp, chdir)
#include <sys/wait.h>   // Wait for process termination
#include <fcntl.h>      // File control options
#include <errno.h>      // Error handling
#include <time.h>       // Time functions

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64

// Function to parse a command string into arguments
void parse_command(char* command, char** args) {
    char* token;
    int i = 0;

    token = strtok(command, " \t\n");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}

// Function to execute a command
void execute_command(char** args) {
    pid_t pid = fork();     // Forking a child process

    if (pid == -1) {
        perror("fork");     // Error handling for fork failure
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {    // Child process
        execvp(args[0], args);   // Execute the command
        perror("execvp");   // Error handling if execvp fails
        exit(EXIT_FAILURE);
    }
    else {                  // Parent process
        int status;
        waitpid(pid, &status, 0);   // Wait for child process to finish
    }
}

// Function to display calendar
void display_calendar(int month, int year) {
    char command[1024];
    
    if (month < 1 || month > 12 || year < 1) {
        fprintf(stderr, "Invalid month or year.\n");
        return;
    }

    sprintf(command, "cal %d %d", month, year);
    system(command);
}

int main() {
    char command[MAX_COMMAND_LENGTH];   // Buffer for user input command
    char* args[MAX_ARGS];               // Array to store command arguments

    // Shell loop
    while (1) {
        printf("Seifyom> ");
        fflush(stdout);         // Flush standard output

        if (fgets(command, sizeof(command), stdin) == NULL) {   // Read user input
            break;  // Exit loop on EOF (Ctrl+D)
        }

        command[strcspn(command, "\n")] = '\0';

        parse_command(command, args);   // Parse command into arguments

        if (args[0] != NULL) {
            // Exit Command
            if (strcmp(args[0], "exit") == 0) {
                printf("Exiting shell.\n");
                return 0;
            }
            // Change Directory Command
            else if (strcmp(args[0], "cd") == 0) {
                if (args[1] != NULL) {
                    if (chdir(args[1]) != 0) {
                        perror("cd");
                    }
                }
                else {
                    fprintf(stderr, "cd: missing argument\n");
                }
            }
            // echo->say
            else if (strcmp(args[0], "say") == 0) {
                if (args[1] != NULL) {
                    printf("%s\n", args[1]);
                }
                else {
                    fprintf(stderr, "say: missing message\n");
                }
            }
            // gedit->open
            else if (strcmp(args[0], "open") == 0) {
                if (args[1] != NULL) {
                    FILE *file = fopen(args[1], "a");
                    fclose(file);
                    
                    char command[1024];
                    sprintf(command, "open %s &", args[1]);
                    system(command);
                }
                else {
                    fprintf(stderr, "open: missing filename\n");
                }
            }
            // cat->display
            else if (strcmp(args[0], "display") == 0) {
                if (args[1] != NULL) {
                    char command[1024];
                    sprintf(command, "cat %s", args[1]);
                    system(command);
                }
                else {
                    fprintf(stderr, "display: missing filename\n");
                }
            }
            // ./a.out->./show.out
            else if (strcmp(args[0], "./show.out") == 0) {
                execute_command(args);
            }
            // cal->calendar
            else if (strcmp(args[0], "calendar") == 0) {
                if (args[1] != NULL && args[2] != NULL) {
                    int month = atoi(args[1]);
                    int year = atoi(args[2]);
                    display_calendar(month, year);
                }
                else if (args[1] != NULL) {
                    int year = atoi(args[1]);
                    if (year >= 1) {
                        for (int month = 1; month <= 12; month++) {
                            display_calendar(month, year);
                        }
                    }
                    else {
                        fprintf(stderr, "calendar: invalid year\n");
                    }
                }
                else {
                    fprintf(stderr, "calendar: missing arguments\n");
                }
            }
            // Input and Output Redirection
            else {
                int redirect_input = 0, redirect_output = 0;    // Flags for input and output redirection
                char* input_file = NULL;
                char* output_file = NULL;

                // Check for input and output redirection symbols
                for (int i = 0; args[i] != NULL; i++) {
                    if (strcmp(args[i], "<") == 0) {
                        redirect_input = 1;    // Set flag for input redirection
                        input_file = args[i + 1];   // Store input file name
                        args[i] = NULL;     // Remove redirection symbol and file name from arguments
                    }
                    else if (strcmp(args[i], ">") == 0) {
                        redirect_output = 1;   // Set flag for output redirection
                        output_file = args[i + 1];  // Store output file name
                        args[i] = NULL;     // Remove redirection symbol and file name from arguments
                    }
                }

                // Perform input redirection if requested
                if (redirect_input) {
                    int fd = open(input_file, O_RDONLY);   // Open input file for reading
                    if (fd == -1) {
                        perror("open");   // Error handling for file open failure
                        continue;   // Continue to next iteration of the loop
                    }
                    dup2(fd, STDIN_FILENO);   // Redirect standard input to file descriptor
                    close(fd);   // Close file descriptor
                }

                // Perform output redirection if requested
                if (redirect_output) {
                    int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);   // Open output file for writing (create if not exist, truncate if exists)
                    if (fd == -1) {
                        perror("open");   // Error handling for file open failure
                        continue;   // Continue to next iteration of the loop
                    }
                    dup2(fd, STDOUT_FILENO);   // Redirect standard output to file descriptor
                    close(fd);   // Close file descriptor
                }

                execute_command(args);   // Execute the command
            }
        }
    }

    printf("Exiting shell.\n");
    return 0;
}
