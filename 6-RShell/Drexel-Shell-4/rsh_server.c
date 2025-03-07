
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>

//INCLUDES for extra credit
//#include <signal.h>
//#include <pthread.h>
//-------------------------

#include "dshlib.h"
#include "rshlib.h"


/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 * 
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port  
 * 
 *      is_threded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 * 
 *      This function basically runs the server by: 
 *          1. Booting up the server
 *          2. Processing client requests until the client requests the
 *             server to stop by running the `stop-server` command
 *          3. Stopping the server. 
 * 
 *      This function is fully implemented for you and should not require
 *      any changes for basic functionality.  
 * 
 *      IF YOU IMPLEMENT THE MULTI-THREADED SERVER FOR EXTRA CREDIT YOU NEED
 *      TO DO SOMETHING WITH THE is_threaded ARGUMENT HOWEVER.  
 */
int start_server(char *ifaces, int port, int is_threaded){
    (void)is_threaded;
    int svr_socket;
    int rc;

    //
    //TODO:  If you are implementing the extra credit, please add logic
    //       to keep track of is_threaded to handle this feature
    //

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0){
        int err_code = svr_socket;  //server socket will carry error code
        return err_code;
    }

    rc = process_cli_requests(svr_socket);

    stop_server(svr_socket);


    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server()
 *                  function. 
 * 
 *      This function simply returns the value of close() when closing
 *      the socket.  
 */
int stop_server(int svr_socket){
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  They are passed
 *                      as is to this function.   
 * 
 *      This function "boots" the rsh server.  It is responsible for all
 *      socket operations prior to accepting client connections.  Specifically: 
 * 
 *      1. Create the server socket using the socket() function. 
 *      2. Calling bind to "bind" the server to the interface and port
 *      3. Calling listen to get the server ready to listen for connections.
 * 
 *      after creating the socket and prior to calling bind you might want to 
 *      include the following code:
 * 
 *      int enable=1;
 *      setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
 * 
 *      when doing development you often run into issues where you hold onto
 *      the port and then need to wait for linux to detect this issue and free
 *      the port up.  The code above tells linux to force allowing this process
 *      to use the specified port making your life a lot easier.
 * 
 *  Returns:
 * 
 *      server_socket:  Sockets are just file descriptors, if this function is
 *                      successful, it returns the server socket descriptor, 
 *                      which is just an integer.
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code is returned if the socket(),
 *                               bind(), or listen() call fails. 
 * 
 */
int boot_server(char *ifaces, int port){
    int svr_socket;
    struct sockaddr_in server_addr;
    int enable = 1;

    // Create the socket
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        return ERR_RDSH_COMMUNICATION;
    }
    
    // Allow reuse of the address
    if (setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    // Setup the address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ifaces, &server_addr.sin_addr) <= 0) {
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    // Bind the socket
    if (bind(svr_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    // Listen with a backlog of 5
    if (listen(svr_socket, 5) < 0) {
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }
    
    return svr_socket;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 *   
 *  This function handles managing client connections.  It does this using
 *  the following logic
 * 
 *      1.  Starts a while(1) loop:
 *  
 *          a. Calls accept() to wait for a client connection. Recall that 
 *             the accept() function returns another socket specifically
 *             bound to a client connection. 
 *          b. Calls exec_client_requests() to handle executing commands
 *             sent by the client. It will use the socket returned from
 *             accept().
 *          c. Loops back to the top (step 2) to accept connecting another
 *             client.  
 * 
 *          note that the exec_client_requests() return code should be
 *          negative if the client requested the server to stop by sending
 *          the `stop-server` command.  If this is the case step 2b breaks
 *          out of the while(1) loop. 
 * 
 *      2.  After we exit the loop, we need to cleanup.  Dont forget to 
 *          free the buffer you allocated in step #1.  Then call stop_server()
 *          to close the server socket. 
 * 
 *  Returns:
 * 
 *      OK_EXIT:  When the client sends the `stop-server` command this function
 *                should return OK_EXIT. 
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code terminates the loop and is
 *                returned from this function in the case of the accept() 
 *                function failing. 
 * 
 *      OTHERS:   See exec_client_requests() for return codes.  Note that positive
 *                values will keep the loop running to accept additional client
 *                connections, and negative values terminate the server. 
 * 
 */
int process_cli_requests(int svr_socket) {
    int cli_socket;
    int rc;
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    
    while (1) {
        cli_socket = accept(svr_socket, (struct sockaddr *)&client_addr, &addrlen);
        if (cli_socket < 0) {
            return ERR_RDSH_COMMUNICATION;
        }
        
        printf("Accepted client connection...\n");
        rc = exec_client_requests(cli_socket);
        
        // If the client sent "stop-server", rc would be OK_EXIT, which stops the server.
        if (rc == OK_EXIT) {
            break;    // Stop the server.
        }
    }
    
    stop_server(cli_socket);
    return OK_EXIT;
}

/*
 * exec_client_requests(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 *   
 *  This function handles accepting remote client commands. The function will
 *  loop and continue to accept and execute client commands.  There are 2 ways
 *  that this ongoing loop accepting client commands ends:
 * 
 *      1.  When the client executes the `exit` command, this function returns
 *          to process_cli_requests() so that we can accept another client
 *          connection. 
 *      2.  When the client executes the `stop-server` command this function
 *          returns to process_cli_requests() with a return code of OK_EXIT
 *          indicating that the server should stop. 
 * 
 *  Note that this function largely follows the implementation of the
 *  exec_local_cmd_loop() function that you implemented in the last 
 *  shell program deliverable. The main difference is that the command will
 *  arrive over the recv() socket call rather than reading a string from the
 *  keyboard. 
 * 
 *  This function also must send the EOF character after a command is
 *  successfully executed to let the client know that the output from the
 *  command it sent is finished.  Use the send_message_eof() to accomplish 
 *  this. 
 * 
 *  Of final note, this function must allocate a buffer for storage to 
 *  store the data received by the client. For example:
 *     io_buff = malloc(RDSH_COMM_BUFF_SZ);
 *  And since it is allocating storage, it must also properly clean it up
 *  prior to exiting.
 * 
 *  Returns:
 * 
 *      OK:       The client sent the `exit` command.  Get ready to connect
 *                another client. 
 *      OK_EXIT:  The client sent `stop-server` command to terminate the server
 * 
 *      ERR_RDSH_COMMUNICATION:  A catch all for any socket() related send
 *                or receive errors. 
 */
int exec_client_requests(int cli_socket) {
    char *io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (!io_buff) {
        return ERR_RDSH_COMMUNICATION;
    }
    
    while (1) {
        memset(io_buff, 0, RDSH_COMM_BUFF_SZ);
        int total_received = 0;
        int recv_bytes;
        int is_last_chunk = 0;
        
        // Receive data until we see the EOF marker
        while (!is_last_chunk) {
            recv_bytes = recv(cli_socket, io_buff + total_received, RDSH_COMM_BUFF_SZ - total_received, 0);
            if (recv_bytes < 0) {
                free(io_buff);
                return ERR_RDSH_COMMUNICATION;
            }
            if (recv_bytes == 0) {
                // Connection closed by client
                free(io_buff);
                return OK;
            }
            total_received += recv_bytes;
            if (io_buff[total_received - 1] == RDSH_EOF_CHAR)
                is_last_chunk = 1;
        }
        
        // Remove the EOF marker by replacing it with a '\0'
        io_buff[total_received - 1] = '\0';
        printf("Received command: [%s]\n", io_buff);

        // Special handling for "exit" and "stop-server"
        if (strcmp(io_buff, "exit") == 0) {
            // "exit" should only close this client connection.
            printf(RCMD_MSG_CLIENT_EXITED);
            send_message_string(cli_socket, "exiting...\n");
            free(io_buff);
            return OK;  // End this client session; server keeps running.
        }
        if (strcmp(io_buff, "stop-server") == 0) {
            printf(RCMD_MSG_SVR_STOP_REQ);
            send_message_string(cli_socket, "exiting...\n");
            free(io_buff);
            return OK_EXIT;  // This code signals process_cli_requests() to break and stop the server.
        }
        
        // Process the commands 
        command_list_t cmd_list;
        int build_status = build_cmd_list(io_buff, &cmd_list);
        if (build_status != OK) {
            free(io_buff);
            return build_status;
        }
        
        // Check if the first command is built-in by using the functions from dshlib.c.
        Built_In_Cmds bi = match_command(cmd_list.commands[0].argv[0]);
        if (bi != BI_NOT_BI) {
            // Built-in command was matched
            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                // Redirect stdout and stderr to client socket
                if (dup2(cli_socket, STDOUT_FILENO) < 0) {
                    exit(errno);
                }
                if (dup2(cli_socket, STDERR_FILENO) < 0) {
                    exit(errno);
                }
                
                // Execute the built-in command
                int bi_rc = exec_built_in_cmd(&cmd_list.commands[0]);
                exit(bi_rc);  // Exit with the built-in command's return code
            } else if (pid > 0) {
                // Parent process
                int status;
                waitpid(pid, &status, 0);
                
                // Send EOF marker to signal end-of-output
                send_message_eof(cli_socket);
                free_cmd_list(&cmd_list);
            } else {
                // Fork failed
                char error_msg[] = "Fork failed for built-in command\n";
                send_message_string(cli_socket, error_msg);
                free_cmd_list(&cmd_list);
            }
        } else {
            // Not a built-in command. Execute as external command or pipeline.
            rsh_execute_pipeline(cli_socket, &cmd_list);
            // Instead of sending rc_msg, just send the EOF marker to signal end-of-output.
            send_message_eof(cli_socket);
            free_cmd_list(&cmd_list);
        }
    }
    
    free(io_buff);
    return OK; // unreachable
}

/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client

 *  Sends the EOF character to the client to indicate that the server is
 *  finished executing the command that it sent. 
 * 
 *  Returns:
 * 
 *      OK:  The EOF character was sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the EOF character. 
 */
int send_message_eof(int cli_socket) {
    int bytes_sent = send(cli_socket, &RDSH_EOF_CHAR, 1, 0);
    if (bytes_sent != 1) {
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}

/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client. 
 *   
 *  Sends a message to the client.  Note this command executes both a send()
 *  to send the message and a send_message_eof() to send the EOF character to
 *  the client to indicate command execution terminated. 
 * 
 *  Returns:
 * 
 *      OK:  The message in buff followed by the EOF character was 
 *           sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the message followed by the EOF character. 
 */
int send_message_string(int cli_socket, char *buff) {
    int len = strlen(buff);
    int bytes_sent = send(cli_socket, buff, len, 0);
    if (bytes_sent != len) {
        fprintf(stderr, CMD_ERR_RDSH_SEND, bytes_sent, len);
        return ERR_RDSH_COMMUNICATION;
    }
    
    return send_message_eof(cli_socket);
}


/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure that we implemented in
 *                   the last shell. 
 *   
 *  This function executes the command pipeline.  It should basically be a
 *  replica of the execute_pipeline() function from the last deliverable. 
 *  The only thing different is that you will be using the cli_sock as the
 *  main file descriptor on the first executable in the pipeline for STDIN,
 *  and the cli_sock for the file descriptor for STDOUT, and STDERR for the
 *  last executable in the pipeline.  See picture below:  
 * 
 *      
 *┌───────────┐                                                    ┌───────────┐
 *│ cli_sock  │                                                    │ cli_sock  │
 *└─────┬─────┘                                                    └────▲──▲───┘
 *      │   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐  │  │    
 *      │   │   Process 1  │     │   Process 2  │     │   Process N  │  │  │    
 *      │   │              │     │              │     │              │  │  │    
 *      └───▶stdin   stdout├─┬──▶│stdin   stdout├─┬──▶│stdin   stdout├──┘  │    
 *          │              │ │   │              │ │   │              │     │    
 *          │        stderr├─┘   │        stderr├─┘   │        stderr├─────┘    
 *          └──────────────┘     └──────────────┘     └──────────────┘   
 *                                                      WEXITSTATUS()
 *                                                      of this last
 *                                                      process to get
 *                                                      the return code
 *                                                      for this function       
 * 
 *  Returns:
 * 
 *      EXIT_CODE:  This function returns the exit code of the last command
 *                  executed in the pipeline.  If only one command is executed
 *                  that value is returned.  Remember, use the WEXITSTATUS()
 *                  macro that we discussed during our fork/exec lecture to
 *                  get this value. 
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    pid_t pids[CMD_MAX] = {0};
    int prev_fd = cli_sock;  // Start with client socket as input
    int rc = OK;

    for (int i = 0; i < clist->num; i++) {
        int pipefd[2] = { -1, -1 };

        // Create a new pipe for all but the last command
        if (i < clist->num - 1) {
            if (pipe(pipefd) < 0) {
                return ERR_MEMORY;
            }
        }

        pids[i] = fork();
        if (pids[i] < 0) {
            return ERR_EXEC_CMD;
        }

        if (pids[i] == 0) { 
            // Child process
            if (prev_fd != -1) {
                if (dup2(prev_fd, STDIN_FILENO) < 0) {
                    exit(errno);
                }
                if (prev_fd != cli_sock) {  // Don't close cli_sock if it's being used for input
                    close(prev_fd);
                }
            }

            if (i < clist->num - 1) {
                // Not the last command: pipe output to next command
                if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
                    exit(errno);
                }
                close(pipefd[0]);
                close(pipefd[1]);
            } else {
                // Last command: redirect output to client socket
                if (dup2(cli_sock, STDOUT_FILENO) < 0) {
                    exit(errno);
                }
                if (dup2(cli_sock, STDERR_FILENO) < 0) {
                    exit(errno);
                }
            }

            // Apply any command-specific redirection
            do_redirection(&clist->commands[i]);

            // If the command is built-in, execute it
            if (match_command(clist->commands[i].argv[0]) != BI_NOT_BI) {
                exec_built_in_cmd(&clist->commands[i]);
                exit(0);
            }

            // Execute the external command
            if (execvp(clist->commands[i].argv[0], clist->commands[i].argv) < 0) {
                exit(errno);
            }
            exit(0);
        } else {
            // Parent process
            if (prev_fd != -1 && prev_fd != cli_sock) {
                close(prev_fd);
            }
            if (i < clist->num - 1) {
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
                    // Send error messages back to client through socket
                    char error_msg[256];
                    switch (rc) {
                        case ENOENT:
                            snprintf(error_msg, sizeof(error_msg), "Error: Command not found in PATH\n");
                            write(cli_sock, error_msg, strlen(error_msg));
                            break;
                        case EACCES:
                            snprintf(error_msg, sizeof(error_msg), "Error: Permission denied\n");
                            write(cli_sock, error_msg, strlen(error_msg));
                            break;
                        default:
                            snprintf(error_msg, sizeof(error_msg), "Command execution failed with code %d\n", rc);
                            write(cli_sock, error_msg, strlen(error_msg));
                            break;
                    }
                }
            }
        } else {
            char error_msg[] = CMD_ERR_EXECUTE;
            write(cli_sock, error_msg, strlen(error_msg));
            rc = ERR_EXEC_CMD;
        }
    }
    return rc;
}