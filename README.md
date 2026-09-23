# Redis-style server in C++

A Redis-style in-memory server in C++, built from first principles while studying systems and network programming.

It doesn't speak the Redis protocol (RESP) yet and has no key-value store yet. Today it is a TCP server and client that exchange length-prefixed messages; the roadmap below shows what comes next.

This is a learning implementation based on *Build Your Own Redis with C/C++* and focuses on understanding socket programming, network protocols, concurrent I/O, and the internal architecture of an in-memory key-value server.

## Roadmap

### Part 1 — Redis from 0 to 1

- [x] Socket programming
- [x] TCP server and client
- [x] Binary request-response protocol and message framing
- [x] Concurrent I/O models
- [ ] Event loop
- [ ] Key-value server

### Part 2 — Advanced Topics

- [ ] Hashtable implementation
- [ ] Data serialization
- [ ] Balanced binary tree
- [ ] Sorted set
- [ ] Timers and timeouts
- [ ] Cache expiration with TTL
- [ ] Thread pool

## Build and run

Needs a C++17 compiler on Linux or macOS (POSIX sockets).

```bash
g++ -std=c++17 -Wall -Wextra redis_server.cpp -o server
g++ -std=c++17 -Wall -Wextra redis_client.cpp -o client

./server        # terminal 1: listens on port 1234
./client        # terminal 2: sends "hello1" and "hello2"
```

Expected output:

```text
# server                  # client
Client says: hello1       Server says: World
Client says: hello2       Server says: World
EOF
```

`EOF` is the server noticing that the client closed the connection; it then goes back to `accept()` for the next client.

## Protocol

Every message, in both directions, is a 4-byte length followed by that many bytes of payload:

```text
+----------------+----------------------+
| len (4 bytes)  | payload (len bytes)  |
+----------------+----------------------+
```

- The length is copied with `memcpy` in host byte order, so it is little-endian on x86 and ARM. Both ends run on the same kind of machine, so this is fine for now.
- Payloads larger than 4096 bytes (`k_max_msg`) are rejected and the connection is closed.
- TCP is a byte stream, not a message stream: one `read()` can return part of a message, or more than one. `read_full()` and `write_all()` loop until exactly `n` bytes have been transferred, so a message is never processed half-read.

## Current status

- The server handles one connection at a time with blocking I/O: while one client is connected, others wait in the listen backlog.
- The next milestone, the event loop, replaces this with non-blocking sockets and `poll()`, so a single thread can serve many clients at once.

## Project Structure

- `redis_server.cpp` — TCP server: `socket` / `bind` / `listen` / `accept`, then `one_request()` in a loop per connection
- `redis_client.cpp` — TCP client: connects to `127.0.0.1:1234` and sends two framed queries

## Rough Outline

```
SERVER (one process)                         CLIENT (one process)
────────────────────                         ────────────────────
socket() / bind() / listen()
     │
     ▼
accept()  ◄──── blocks, waiting ───────────  socket()
     │                                             │
     │                                        connect()
     │◄─────────── TCP connection ────────────────┘
     │  (accept returns connfd)                    │
     │                                             │
┌────▼── inner loop ──────┐              ┌─────────▼── query("hello1") ──────┐
│ one_request(connfd):    │              │ build [len|"hello1"]              │
│                         │   4B len +   │ write_all(fd, ...) ──────────────►│ (bytes travel over socket)
│ read_full(rbuf, 4)  ◄───┼── payload ───┤                                   │
│ memcpy → len            │              │                                   │
│ read_full(&rbuf[4],len) │              │                                   │
│ printf "client says:    │              │                                   │
│        hello1"          │              │                                   │
│                         │   4B len +   │ read_full(rbuf, 4)  ◄─────────────┤
│ build [len|"world"]     │── payload ──►│ memcpy → len                      │
│ write_all(connfd,...) ──┼──────────────┤ read_full(&rbuf[4], len)          │
│ return 0                │              │ printf "server says: world"       │
└────┬────────────────────┘              └───────────────────────────────────┘
     │  loops back                               │  query() returns 0
     │                                           ▼
┌────▼── one_request again ┐              ┌── query("hello2") ── (same round-trip) ──┐
│ ... "client says:hello2" │◄────────────►│ ... "server says: world"                 │
│ replies "world" again    │              └──────────────────────┬───────────────────┘
└────┬─────────────────────┘                                     │
     │                                                    close(fd)  ◄── goto L_DONE
     │  next read_full gets 0 bytes                              │
     ▼                                                    (client exits)
read_full returns -1
one_request returns err
inner loop breaks → prints "EOF"
close(connfd); accept() again
```

## Rough Outline

```
SERVER (one process)                         CLIENT (one process)
────────────────────                         ────────────────────
socket() / bind() / listen()
     │
     ▼
accept()  ◄──── blocks, waiting ───────────  socket()
     │                                             │
     │                                        connect()
     │◄─────────── TCP connection ────────────────┘
     │  (accept returns connfd)                    │
     │                                             │
┌────▼── inner loop ──────┐              ┌─────────▼── query("hello1") ──────┐
│ one_request(connfd):    │              │ build [len|"hello1"]              │
│                         │   4B len +   │ write_all(fd, ...) ──────────────►│ (bytes travel over socket)
│ read_full(rbuf, 4)  ◄───┼── payload ───┤                                   │
│ memcpy → len            │              │                                   │
│ read_full(&rbuf[4],len) │              │                                   │
│ printf "client says:    │              │                                   │
│        hello1"          │              │                                   │
│                         │   4B len +   │ read_full(rbuf, 4)  ◄─────────────┤
│ build [len|"world"]     │── payload ──►│ memcpy → len                      │
│ write_all(connfd,...) ──┼──────────────┤ read_full(&rbuf[4], len)          │
│ return 0                │              │ printf "server says: world"       │
└────┬────────────────────┘              └───────────────────────────────────┘
     │  loops back                               │  query() returns 0
     │                                           ▼
┌────▼── one_request again ┐              ┌── query("hello2") ── (same round-trip) ──┐
│ ... "client says:hello2" │◄────────────►│ ... "server says: world"                 │
│ replies "world" again    │              └──────────────────────┬───────────────────┘
└────┬─────────────────────┘                                     │
     │                                                    close(fd)  ◄── goto L_DONE
     │  next read_full gets 0 bytes                              │
     ▼                                                    (client exits)
read_full returns -1
one_request returns err
inner loop breaks → prints "EOF"
close(connfd); accept() again
```

## Learning Goals

- TCP socket programming in C++
- Message framing over byte streams
- Handling partial reads and writes
- Network protocol design and parsing
- Concurrent I/O and event loops
- In-memory data structures and key-value storage

## Reference

This project follows *Build Your Own Redis with C/C++* from build-your-own.org as a guided systems programming study.
