1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

    > **Answer**: The remote client determines when a command's output is fully received by implementing a message framing protocol, where each transmission includes a header containing the message length and type. The client uses a complete read function that handles partial reads by continuing to read until the entire message is received, as indicated by the header's length field. When receiving command output, the client enters a loop that first reads the header, then reads the corresponding data in chunks. This continues until an EOF message is received, signaling the end of the command's output.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

    > **Answer**: A networked shell protocol should implement message framing to handle TCP's stream-oriented nature, where each message includes a header containing the length and type of the following data. This header allows the receiver to know exactly how much data to expect and when a complete command or response has been received. Without proper framing, critical issues can arise such as commands running together, partial command execution, buffer overflows, and the inability to match responses with their corresponding commands.

3. Describe the general differences between stateful and stateless protocols.

    > **Answer**: Stateful protocols maintain information about previous interactions between client and server across multiple requests, like keeping track of a user's login session or command history. The server must allocate resources to store this state information and handle scenarios like connection drops or server restarts. In contrast, stateless protocols treat each request as independent and self-contained, requiring no server-side storage of client information between requests.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

    > **Answer**: UDP is used despite being "unreliable" because it offers significant advantages in specific scenarios. It provides faster transmission by eliminating the overhead of TCP's connection establishment, acknowledgments, and ordered delivery guarantees. This makes UDP ideal for real-time applications like video streaming, or online gaming where speed is more important than perfect reliability, and where occasional packet loss is acceptable.

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

    > **Answer**: The operating system provides the socket interface/abstraction for network communications. Sockets act as endpoints for communication between applications, offering a file descriptor-like API that allows programs to perform network operations using familiar read/write operations. 