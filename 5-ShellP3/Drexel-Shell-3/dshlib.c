#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#include "dshlib.h"

// The Drexel Dragon ASCII art print function from dragon.c file
extern void print_dragon();

// Global variable to store the return code of the last command
int last_rc = 0;

// Helper function to trim leading and trailing spaces
char* trim_whitespace(char* str) {
    // Trim leading space
    while (*str == SPACE_CHAR) {
        str++;
    }

    // Trim trailing space
    char *end = str + strlen(str) - 1;
    while (end > str && *end == SPACE_CHAR) {
        end--;
    }
    *(end + 1) = '\0';

    return str;
}

// Allocate memory for the command buffer
int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX * sizeof(char));
    if (!cmd_buff->_cmd_buffer) {
        return ERR_MEMORY;
    }
    memset(cmd_buff->_cmd_buffer, 0, SH_CMD_MAX);
    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));

    // Initialize redirection fields
    cmd_buff->input_file = NULL;
    cmd_buff->output_file = NULL;
    cmd_buff->out_append = 0;

    return OK;
}

// Free the memory allocated for the command buffer
int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    return OK;
}

// Clear the command buffer to be reused to hold new command
int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        memset(cmd_buff->_cmd_buffer, 0, SH_CMD_MAX);
    }
    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
    cmd_buff->input_file = NULL;
    cmd_buff->output_file = NULL;
    cmd_buff->out_append = 0;
    return OK;
}

// Build the command buffer from the command line
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    clear_cmd_buff(cmd_buff);

    // Trim leading and trailing spaces
    char *trimmed = trim_whitespace(cmd_line);

    // Copy the trimmed command line to cmd_buff->_cmd_buffer
    strncpy(cmd_buff->_cmd_buffer, trimmed, SH_CMD_MAX - 1);
    cmd_buff->_cmd_buffer[SH_CMD_MAX - 1] = '\0';

    int argc = 0;
    char *ptr = trimmed;
    bool in_quotes = false;
    char *start = NULL;

    while (*ptr) {
        if (!in_quotes && isspace((char)*ptr)) {
            ptr++;
            continue;
        }

        if (*ptr == '\"') {
            in_quotes = !in_quotes;
            if (in_quotes) {
                start = ptr + 1;
            } else {
                *ptr = '\0';
                if (argc < CMD_MAX) {
                    cmd_buff->argv[argc++] = start;
                }
            }
            ptr++;
            continue;
        }

        if (!in_quotes) {
            start = ptr;
            while (*ptr && !isspace((char)*ptr)) {
                if (*ptr == '\"') {
                    break;
                }
                ptr++;
            }
            if (*ptr) {
                *ptr = '\0';
                ptr++;
            }
            if (argc < CMD_MAX) {
                cmd_buff->argv[argc++] = start;
            }
        } else {
            // Inside quotes, treat everything as a single argument
            start = ptr;
            while (*ptr && *ptr != '\"') {
                ptr++;
            }
            if (*ptr == '\"') {
                *ptr = '\0';
                in_quotes = false;
                ptr++;
            }
            if (argc < CMD_MAX) {
                cmd_buff->argv[argc++] = start;
            }
        }
    }

    cmd_buff->argc = argc;
    cmd_buff->argv[argc] = NULL;

    // Check for redirection operators
    int new_argc = 0;
    for (int i = 0; i < argc; i++) {
        if (strcmp(cmd_buff->argv[i], "<") == 0) {
            if (i + 1 < argc) {
                cmd_buff->input_file = cmd_buff->argv[i+1];
                i++; // Skip filename
            }
        } else if (strcmp(cmd_buff->argv[i], ">") == 0) {
            if (i + 1 < argc) {
                cmd_buff->output_file = cmd_buff->argv[i+1];
                cmd_buff->out_append = 0;
                i++;
            }
        } else if (strcmp(cmd_buff->argv[i], ">>") == 0) {
            if (i + 1 < argc) {
                cmd_buff->output_file = cmd_buff->argv[i+1];
                cmd_buff->out_append = 1;
                i++;
            }
        } else {
            cmd_buff->argv[new_argc++] = cmd_buff->argv[i];
        }
    }
    cmd_buff->argv[new_argc] = NULL;
    cmd_buff->argc = new_argc;

    return OK;
}

// Build the command list from the command line
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    clist->num = 0;
    char *token = strtok(cmd_line, PIPE_STRING);

    while (token && clist->num < CMD_MAX) {
        token = trim_whitespace(token);
        if (strlen(token) > 0) {
            alloc_cmd_buff(&(clist->commands[clist->num]));
            build_cmd_buff(token, &(clist->commands[clist->num]));
            clist->num++;
        }
        token = strtok(NULL, PIPE_STRING);
    }

    if (clist->num == CMD_MAX) {
        free_cmd_list(clist);
        return ERR_TOO_MANY_COMMANDS;
    }

    if (clist->num == 0) {
        return WARN_NO_CMDS;
    }

    return OK;
}

// Free the memory allocated for the command list
int free_cmd_list(command_list_t *cmd_lst) {
    for (int i = 0; i < cmd_lst->num; i++) {
        free_cmd_buff(&cmd_lst->commands[i]);
    }
    cmd_lst->num = 0;
    return OK;
}

// Built-in command matching function
Built_In_Cmds match_command(const char *input) {
    if (strcmp(input, "exit") == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input, "dragon") == 0) {
        return BI_CMD_DRAGON;
    } else if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    } else if (strcmp(input, "rc") == 0) {
        return BI_RC;
    }
    return BI_NOT_BI;
}

// Built-in command execution function
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {

    Built_In_Cmds cmd_type = match_command(cmd->argv[0]);
    int rc = BI_NOT_BI;

    // Only "dragon" and "rc" should run with redirection in a child
    if ((cmd_type == BI_CMD_DRAGON || cmd_type == BI_RC) &&
         (cmd->input_file != NULL || cmd->output_file != NULL)) {
        pid_t pid = fork();
        if (pid == 0) {
            // In the child process, apply redirection then execute the built-in
            do_redirection(cmd);
            if (cmd_type == BI_CMD_DRAGON) {
                print_dragon();
            } else if (cmd_type == BI_RC) {
                printf("%d\n", last_rc);
            }
            exit(0);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            rc = BI_EXECUTED;
        }
    } else {
        switch (cmd_type) {
            case BI_CMD_EXIT:
                rc = BI_CMD_EXIT;
                break;
            case BI_CMD_DRAGON:
                print_dragon();
                rc = BI_EXECUTED;
                break;
            case BI_CMD_CD:
                if (cmd->argc == 1) {
                    rc = BI_EXECUTED;
                } else if (cmd->argc == 2) {
                    if (chdir(cmd->argv[1]) != 0) {
                        printf(CD_DIR_ERR, cmd->argv[1]);
                    }
                    rc = BI_EXECUTED;
                } else {
                    printf(CD_TOO_MANY_ARGS);
                    rc = BI_EXECUTED;
                }
                break;
            case BI_RC:
                printf("%d\n", last_rc);
                rc = BI_EXECUTED;
                break;
            default:
                rc = BI_NOT_BI;
                break;
        }
    }
    return rc;
}

void do_redirection(cmd_buff_t *cmd) {
    if (cmd->input_file) {
        int fd_in = open(cmd->input_file, O_RDONLY);
        if (fd_in < 0) {
            perror("open input");
            exit(errno);
        }
        if (dup2(fd_in, STDIN_FILENO) < 0) {
            perror("dup2 input");
            exit(errno);
        }
        close(fd_in);
    }
    if (cmd->output_file) {
        int flags = O_WRONLY | O_CREAT;
        if (cmd->out_append)
            flags |= O_APPEND;
        else
            flags |= O_TRUNC;
        int fd_out = open(cmd->output_file, flags, 0644);
        if (fd_out < 0) {
            perror("open output");
            exit(errno);
        }
        if (dup2(fd_out, STDOUT_FILENO) < 0) {
            perror("dup2 output");
            exit(errno);
        }
        close(fd_out);
    }
}

// Function to execute the external command
int exec_cmd(cmd_buff_t *cmd) {
    // Fork a child process to execute the command
    pid_t pid = fork();
    if (pid < 0) {
        // Fork failed
        printf(CMD_ERR_EXECUTE);
        return ERR_EXEC_CMD;
    } else if (pid == 0) {
        // Child process
        do_redirection(cmd);
        if (execvp(cmd->argv[0], cmd->argv) < 0) {
            exit(errno);
            return ERR_EXEC_CMD;
        }
    } else {    
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            int rc = WEXITSTATUS(status);
            if (rc != 0) {
                // Determine error type based on errno
                switch (rc) {
                    case ENOENT:
                        printf("Error: Command not found in PATH\n");
                        break;
                    case EACCES:
                        printf("Error: Permission denied\n");
                        break;
                    default:
                        printf("Command execution failed with code %d\n", rc);
                        break;
                }
            }
            return rc;
        } else {
            printf(CMD_ERR_EXECUTE);
            return ERR_EXEC_CMD;
        }
    }

    return OK;
}

// Function to execute the pipeline
int exec_pipeline(command_list_t *clist) {
    pid_t pids[CMD_MAX] = {0};
    int prev_fd = -1;  // read end of previous pipe
    int rc = OK;

    for (int i = 0; i < clist->num; i++) {
        int pipefd[2] = { -1, -1 };
        if (i < clist->num - 1) {
            if (pipe(pipefd) < 0) {
                return ERR_MEMORY;
            }
        }

        pids[i] = fork();
        if (pids[i] < 0) {
            return ERR_EXEC_CMD;
        } else if (pids[i] == 0) {
            // Child process
            if (prev_fd != -1) {
                if (dup2(prev_fd, STDIN_FILENO) < 0) {
                    exit(errno);
                }
            } else if (i < clist->num - 1) {
                if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
                    exit(errno);
                }
            }

            // Close all pipe file descriptors
            if (prev_fd != -1) {
                close(prev_fd);
            } else if (i < clist->num - 1) {
                close(pipefd[0]);
                close(pipefd[1]);
            }

            // Execute the command
            do_redirection(&clist->commands[i]);
            if (match_command(clist->commands[i].argv[0]) != BI_NOT_BI) {
                exec_built_in_cmd(&clist->commands[i]);
                exit(0);
            } else if (execvp(clist->commands[i].argv[0], clist->commands[i].argv) < 0) {
                exit(errno);
            }
            exit(0);
        } else {
            // Parent process
            if (prev_fd != -1) {
                close(prev_fd);
            } else if (i < clist->num - 1) {
                close(pipefd[1]);
                prev_fd = pipefd[0];
            }
        }
    }

    // Parent waits for all children
    for (int i = 0; i < clist->num; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status)) {
            if (i == clist->num - 1) {
                rc = WEXITSTATUS(status);
                if (rc != 0) {
                    // Determine error type based on errno
                    switch (rc) {
                        case ENOENT:
                            printf("Error: Command not found in PATH\n");
                            break;
                        case EACCES:
                            printf("Error: Permission denied\n");
                            break;
                        default:
                            printf("Command execution failed with code %d\n", rc);
                            break;
                    }
                }
            }
        } else {
            printf(CMD_ERR_EXECUTE);
            rc = ERR_EXEC_CMD;
        }
    }
    return rc;
}

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
int exec_local_cmd_loop()
{
    char *cmd_input = malloc(SH_CMD_MAX * sizeof(char));
    int rc = OK;
    command_list_t cmd_list;
    cmd_list.num = 0;

    while (1) {
        printf("%s", SH_PROMPT);

        // Read input from the user
        if (fgets(cmd_input, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        // Remove the trailing \n
        cmd_input[strcspn(cmd_input, "\n")] = '\0';

        // Build command list from input
        rc = build_cmd_list(cmd_input, &cmd_list);

        // Check if no commands were entered
        if (rc == WARN_NO_CMDS) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        // If only one command was parsed, execute it
        if (cmd_list.num == 1) {
            Built_In_Cmds bi = exec_built_in_cmd(&cmd_list.commands[0]);
            if (bi == BI_EXECUTED) {
                rc = OK;
                continue;
            } else if (bi == BI_CMD_EXIT) {
                printf("exiting...\n");
                rc = OK;
                break;
            } else if (bi == BI_NOT_BI) {
                // Execute the external command
                rc = exec_cmd(&cmd_list.commands[0]);
            }
        } else {
            // More than one command => pipeline
            rc = exec_pipeline(&cmd_list);
        }

        // Store the return code of the last command
        last_rc = rc;

        // Free the memory allocated for the command list
        free_cmd_list(&cmd_list);
    }

    free(cmd_input);
    return rc;
}