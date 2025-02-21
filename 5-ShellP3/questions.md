1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

    > **Answer**: My implementation records each child PID and then loops calling `waitpid()` on each one before accepting the next command. Without these `waitpid()` calls, finished child processes would become zombie processes and the shell would continue without guaranteeing that all children finished, which could lead to resource leaks and unpredictable behavior.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

    > **Answer**: After `dup2()` duplicates a file descriptor, the original pipe ends remain open. You must close these unused ends to prevent resource leaks and to ensure that the EOF is correctly sent to processes reading from the pipe. If you leave the pipe ends open, the reader may block indefinitely waiting for EOF, because the pipe still has open file descriptors keeping it active.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

    > **Answer**: The `cd` command is implemented as a built-in because it must change the working directory of the shell process itself. If `cd` were implemented as an external command, it would run in a separate child process, and any directory change would only affect that child. When the child terminates, the parent shell's directory would remain unchanged, making cd ineffective.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

    > **Answer**: To support an arbitrary number of piped commands, I could replace the fixed-size array with a dynamically allocated structure like a linked list that expands as needed. This will allow to resize my command list array when more commands are detected. The trade-offs include increase in complexity in memory management, a decrease in performance due to frequent reallocations, or the increase in difficulty of managing a more complex data structure.
