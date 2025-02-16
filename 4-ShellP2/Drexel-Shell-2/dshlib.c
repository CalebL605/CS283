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
    return OK;
}

// Enhanced build_cmd_buff to handle quoted strings
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
    cmd_buff_t cmd_buff;

    // Allocate memory for cmd_buff
    rc = alloc_cmd_buff(&cmd_buff);
    if (rc == ERR_MEMORY) {
        printf(MEM_ERR);
        return rc;
    }

    while (1) {
        printf("%s", SH_PROMPT);

        // Read input from the user
        if (fgets(cmd_input, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }

        // Remove the trailing \n
        cmd_input[strcspn(cmd_input, "\n")] = '\0';

        // Build the command buffer
        rc = build_cmd_buff(cmd_input, &cmd_buff);

        // Check if no commands were entered
        if (cmd_buff.argc == 0) {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        // Execute the command if it is a built-in command
        rc = exec_built_in_cmd(&cmd_buff);
        if (rc == BI_EXECUTED) {
            rc = OK;
            continue;
        } else if (rc == BI_CMD_EXIT) {
            rc = OK_EXIT;
            break;
        }

        // Execute the external command
        rc = exec_cmd(&cmd_buff);

        // Store the return code of the last command
        last_rc = rc;
    }

    // Free allocated memory
    free(cmd_input);
    free_cmd_buff(&cmd_buff);
    return rc;
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