# A complete Multi-threaded TCP Server for linux

## Description

This project implements a simple multi-threaded TCP server in C using POSIX sockets and pthreads. The server accepts multiple client connections and handles each client in a separate thread.

## Features

* Concurrent client handling using threads
* TCP socket communication
* Echo server functionality
* Graceful client disconnection handling

## Requirements

* Linux / Unix-based system
* GCC compiler
* pthread library

## File Structure

```
.
├── server.c
└── README.md
```

## Compilation

```bash
gcc server.c -o server -lpthread
```

## Running the Server

```bash
./server
```

The server will start listening on port `8080`.

## Testing the Server

### Using Netcat

```bash
nc localhost 8080
```

### Using Telnet

```bash
telnet localhost 8080
```

## How It Works

1. Creates a TCP socket
2. Binds to port 8080
3. Listens for incoming connections
4. Accepts clients
5. Spawns a new thread for each client
6. Each thread:

   * Receives messages
   * Sends responses
   * Closes connection on disconnect

