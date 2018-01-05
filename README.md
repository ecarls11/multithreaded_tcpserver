- This was built and tested on lubuntu 16.10, for unix platforms using C++

- The server was built to handle multiple clients at once, making use of
  std::thread. When the server is started, two threads begin running.

  * A "main server" thread

  * A listener thread

  The main server thread blocks on a message queue (really a wrapper around std::queue with some slight mods), waiting for some sort of message. Right now, it receives messages  from threads that are dynamically created to handle clients indicating that they have finished running and need to be deleted.

  The listener thread blocks on an accept call, waiting for a client to connect. When someone connects, this thread spawns a new thread that services the newly connected client.

- A maximum of 10 clients can connect at one time.

