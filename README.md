# Linux Shell & TCP/IP Chat Application

A multi-client TCP chat server and client built in **C** using **Linux/POSIX APIs** and **socket programming**. The server handles multiple simultaneous clients using `select()` for I/O multiplexing and broadcasts messages across all connected clients.

---

## Features

- Multi-client support (up to 10 simultaneous clients)
- Real-time message broadcasting to all connected clients
- Non-blocking I/O multiplexing using `select()`
- Graceful client disconnect handling
- POSIX-compliant — runs natively on Linux/UNIX

---

## Tech Stack

| Component | Technology |
|---|---|
| Language | C |
| OS | Linux / UNIX |
| APIs | POSIX (sys/socket.h, unistd.h, netinet/in.h) |
| Protocol | TCP/IP (IPv4) |
| Multiplexing | select() |
| Tools | GCC, Git |

---

## Project Structure

```
.
├── server.c      # Multi-client TCP server with broadcast
├── client.c      # TCP client with simultaneous send/receive
└── README.md
```

---

## How to Run

### Prerequisites
- Linux / WSL (Windows Subsystem for Linux)
- GCC compiler

```bash
sudo apt update
sudo apt install gcc -y
```

### Compile

```bash
gcc server.c -o server
gcc client.c -o client
```

### Run

**Terminal 1 — Start the server:**
```bash
./server
```
```
Server listening on port 8080...
```

**Terminal 2 — Connect first client:**
```bash
./client
```
```
Connected to server at 127.0.0.1:8080
Type your message and press Enter:
```

**Terminal 3 — Connect second client:**
```bash
./client
```

Now type a message in Terminal 2 — it appears instantly in Terminal 3. 

---

## How It Works

```
Client 1 ──┐
Client 2 ──┼──► Server (select()) ──► Broadcast to all other clients
Client 3 ──┘
```

1. **Server** creates a TCP socket, binds to port 8080, and listens for connections
2. **`select()`** monitors all client sockets simultaneously without threads
3. When a message arrives from any client, the server **broadcasts** it to all others
4. **Client** uses `select()` to watch both keyboard input and incoming messages at the same time

---

## Key Concepts Demonstrated

- `socket()`, `bind()`, `listen()`, `accept()` — TCP server lifecycle
- `connect()`, `send()`, `recv()` — client communication
- `select()` + `fd_set` — I/O multiplexing for concurrent clients
- `SO_REUSEADDR` — port reuse on server restart
- `inet_pton()` / `inet_ntoa()` — IP address conversion
- Process and file descriptor management in Linux/UNIX

---

## Author

**Ashwani Mishra**  
B.Tech Computer Science — Chandigarh Engineering College, Mohali  
GitHub: [Ash-Cash11](https://github.com/Ash-Cash11)  
LinkedIn: [ashwani-mishra-2oo4](https://linkedin.com/in/ashwani-mishra-2oo4)
