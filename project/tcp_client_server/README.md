# TCP Server & Client

Two minimal programs:

* **server.c** – Listens on TCP port `8181`, prints what the client sends, replies with `"HELLO"`, then exits.
* **client.c** – Connects to `127.0.0.1:8181`, sends `"ping"`, prints the reply, then exits.

Great as a first contact with BSD sockets: `socket → bind → listen → accept` on the server, and `socket → connect` on the client.

---

## 1. Project Layout

```
.
├── server.c   # the listening side
└── client.c   # the connecting side
```

---

## 2. Build

### Linux / macOS

```bash
cc server.c -o server -Wall -Wextra -pedantic -std=c99
cc client.c -o client -Wall -Wextra -pedantic -std=c99
```

*(Use `gcc` or `clang`; the flags are optional but recommended.)*

### Windows (WSL or MSYS2/MinGW)

* **WSL**: treat it like Linux.
* **MSYS2/MinGW**: same commands usually work; this code uses POSIX headers, so pure MSVC needs winsock changes.

---

## 3. Run It

Open **two terminals**.

### Terminal 1 – Start the server

```bash
./server
# Listening on port 8181...
```

### Terminal 2 – Run the client

```bash
./client
# Received: HELLO
```

Server output will look like:

```
Listening on port 8181...
Client connected: 127.0.0.1
Received: ping
```

> If nothing happens, check firewall rules or that both programs run on the same host.

---

## 4. Walkthrough: Server

Core calls in order:

1. **`socket(AF_INET, SOCK_STREAM, 0)`** – Create a TCP socket.
2. **`bind()`** – Attach it to local IP/port (`INADDR_ANY:8181`).
3. **`listen()`** – Mark socket as passive (backlog = 5).
4. **`accept()`** – Block until a client connects; returns a new socket `c`.
5. **`read(c, ...)` / `write(c, ...)`** – Exchange data.
6. **`close(c)` & `close(s)`** – Close client and listening sockets.

Key snippets:

```c
server.sin_family = AF_INET;
server.sin_addr.s_addr = INADDR_ANY;     // any interface
server.sin_port = htons(PORT);           // host→network order

if (bind(s, (struct sockaddr*)&server, sizeof(server)) < 0) { perror("bind"); }
if (listen(s, 5) < 0) { perror("listen"); }

addrlen = sizeof(client);                // must set before accept()
c = accept(s, (struct sockaddr*)&client, &addrlen);
```

---

## 5. Walkthrough: Client

1. **`socket()`** – Same as server.
2. **`inet_pton()`** – Convert string IP to binary for `sockaddr_in`.
3. **`connect()`** – Establish TCP connection.
4. **`write()`** – Send `"ping"`.
5. **`read()`** – Receive `"HELLO"`.
6. **`close()`** – Done.

Snippet:

```c
inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("connect");
}
write(sock, "ping", 4);
int bytes = read(sock, buffer, sizeof(buffer)-1);
```

## 6. Mini Sequence Diagram

```
Client                         Server
  |  socket()                   |
  |---------------------------> |
  |              socket()/bind/listen
  |                             |
  | connect() ----------------> | accept()
  | write("ping") ------------> | read()
  | <-------------------------- write("HELLO")
  | read()                      |
  | close() ------------------> | close(client)
                                | close(listen)
```