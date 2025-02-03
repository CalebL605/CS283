#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char *token = strtok(cmd_line, PIPE_STRING);
    int cmd_count = 0;
    char *commands[CMD_MAX];

    // Split the command line by pipes 
    while (token != NULL) {
        // Check to make sure there are no more than 8 commands
        if (cmd_count > CMD_MAX - 1) {
            return ERR_TOO_MANY_COMMANDS;
        }

        // Remove leading spaces in the command
        while (*token == SPACE_CHAR) {
            token++;
        }

        // Remove trailing spaces in the command
        char *end = token + strlen(token) - 1;
        while (end > token && *end == SPACE_CHAR) {
            end--;
        }
        *(end + 1) = '\0';

        // Store the command and move on to the next one
        commands[cmd_count++] = token;
        token = strtok(NULL, PIPE_STRING);
    }

    // Parse each command into executable and arguments
    clist->num = 0;
    for (int i = 0; i < cmd_count; i++) {
        // Split the first word of the command into executable
        char *exe = strtok(commands[i], " ");
        if (exe == NULL) {
            continue;
        }

        // Check if the executable name is too long
        if (strlen(exe) > EXE_MAX - 1) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        // Copy the executable name into command list
        strncpy(clist->commands[clist->num].exe, exe, EXE_MAX - 1);
        clist->commands[clist->num].exe[EXE_MAX - 1] = '\0';

        // Collect the remaining tokens as arguments
        char *arg = strtok(NULL, "");
        if (arg != NULL) {
            // Remove leading spaces from arguments
            while (*arg == SPACE_CHAR) {
                arg++;
            }

            // Check if the arguments are too long
            if (strlen(arg) > ARG_MAX - 1) {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }

            // Copy the arguments into command list
            strncpy(clist->commands[clist->num].args, arg, ARG_MAX - 1);
            clist->commands[clist->num].args[ARG_MAX - 1] = '\0';
        } 
        else {
            // Argument is empty
            clist->commands[clist->num].args[0] = '\0';
        }

        // Next command
        clist->num++;
    }

    return OK;
}