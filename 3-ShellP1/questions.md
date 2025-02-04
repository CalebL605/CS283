1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**: `fgets()` is a good choice because it helps make sure the input that gets read into the Drexel Shell and gets stored in a buffer does not exceed the allocated buffer size and also leaves a space for the null-terminator char. The function also returns `NULL` if `Ctrl+D` is called allowing it to gracefully handle EOF. 

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**: A large fixed-size array declared inside `main()` would be stored on the stack, which has limited space. Allocating memory on the heap with `malloc()` prevents potential stack overflows.

3. In `dshlib.c`, the function `build_cmd_list()` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**: We must trim leading and trailing spaces from each command before storing it because it might cause execution failures when trying to compare the input with the correct command. For example, the `[ls]` command isn't looking for `[ ls]` or `[ls ]` and this might make it hard for the shell the properly execute. By removing these spaces we can keep things consistent and not allow some unintended behavior to occur by accident. Better safe than sorry.  

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**: 3 redirection examples that we should implement in our custom shell are the output redirection (`>`), the input redirection (`<`), and the appending output redirection (`>>`). A challenge one might face when trying to implement these three redirections are maybe using the correct flags when calling the `open()` function. Another challenge might be trying to handle all the type of errors since there might be conflicts between piping and redirecting, errors opening or accessing files, or errors when trying to get input from stdin, output from stdout, or errors from stderr.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**: Redirection allows the users to redirects the input or output of a command to/from a file instead of the terminal. On the other hand, piping can only pass the output of one command as the input to another command.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**: It is important to keep error messages and regular ouput seperate in a shell because it can be difficult to differentiate between expected results and errors. Another reason to keep them seperate is that it keeps the output clean, and since pipes only pass STDOUT by default, keeping STDERR separate prevents errors from interfering with command chaining.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  If there is an error, our custom shell should print the error message from STDERR to the terminal so the user is aware of the issue and maybe also store into a error log so users can always go back and check the errors for debugging or other purposes. For cases where a command outputs both STDOUT and STDERR, we can allow our custom shell to give the user the possibility to use redirection to merge STDERR to STDOUT or the other way around if the user ever want to see both at the same time.