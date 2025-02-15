1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  We use `fork` before `execvp` because `execvp` replaces the current process with a new program, meaning that if we called `execvp` directly, our original process would no longer exist. By using `fork`, we create a new child process to execute the new program while the parent process remains unchanged.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If the `fork()` system call fails, it indicates that the creation of a new process was unsuccessful. My implementation will stop executing, print that there was an error, return `ERR_EXEC_CMD`, and wait for the next command.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  `execvp()` finds the command to execute by searching for the specified program in the directories listed in the `PATH` environment variable.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  The purpose of calling `wait()` in the parent process after forking is to synchronize with the child process and prevent the creation of zombie processes that consumes system resources. 

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  `WEXITSTATUS()` extracts the exit code of a terminated child process from the status returned by `wait()`. It important because it helps the parent process determine success or failure of the child process and allow the parent to handle the error codes gracefully.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  My implementation of `build_cmd_buff()` handles quoted arguments by treating everything inside quotes ("...") as a single argument, even if it contains spaces. This is necessary because commands like `echo()` that can take arguments with spaces or multiple words will be able to by using quotes. Also, without this logic, `execvp()` would split arguments incorrectly.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  I made a lot of changes in my parsing logic compared to the previous assignment. I had to recode the assignment and use snippets of code from the previous assignment because of how different my two codes were. The trimming of spaces stayed the same, but the difference in data structure in holding the command, the removal of piping and command list summary, and using two separate function for handling built-in and external commands made it super difficult to refactor the old code. There weren't any unexpected challenges, as I already expected the worst. 

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals are a form of IPC in Linux used to notify processes of events such as interrupts, exceptions, or specific conditions. They allow the operating system and applications to communicate asynchronously with running processes. Unlike other IPC mechanisms that involves explicit data exchange and synchronization between processes, signals are simple notifications that can interrupt a process's flow to handle events immediately. 

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  
    
    > **1.**  SIGKILL (Signal 9): This signal forces a process to terminate immediately. It cannot be caught, blocked, or ignored by the process, ensuring termination. It's typically used when a process is unresponsive or refusing to terminate.
    
    > **2.** SIGTERM (Signal 15): This is a termination signal requesting a process to gracefully terminate. Unlike SIGKILL, processes can catch and handle SIGTERM, allowing them to perform cleanup operations before exiting. It's the default signal sent by commands like kill.
    
    > **3.** SIGINT (Signal 2): This signal is sent when a user interrupts a process from the terminal, usually by pressing Ctrl+C. By default, it terminates the process, but processes can catch and handle SIGINT to perform specific actions or ignore it.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  When a process receives the SIGSTOP signal, it is paused and moved to a stopped state. Unlike SIGINT, SIGSTOP cannot be caught, blocked, or ignored by the process. This design ensures that any process can be suspended reliably, allowing for operations like debugging or job control without the process interfering with the stop signal.
