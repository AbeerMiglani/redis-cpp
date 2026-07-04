# Redis-Compatible Server in C++

A Redis-compatible server implementation in C++ built while studying systems and network programming.

This is a learning implementation based on *Build Your Own Redis with C/C++* and focuses on understanding socket programming, network protocols, concurrent I/O, and the internal architecture of an in-memory key-value server.

## Roadmap

### Part 1 — Redis from 0 to 1

- [x] Socket programming
- [x] TCP server and client
- [ ] Binary request-response protocol and message framing
- [ ] Concurrent I/O models
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

## Current Focus

Currently implementing a length-prefixed binary request-response protocol to frame messages over a TCP byte stream.

The protocol uses a 4-byte length prefix followed by a variable-length payload. The implementation also handles partial socket reads and writes rather than assuming a single `read()` or `write()` call transfers an entire message.

## Project Structure

- `redis_server.cpp` — TCP server and request handling
- `redis_client.cpp` — TCP client implementation

## Learning Goals

- TCP socket programming in C++
- Message framing over byte streams
- Handling partial reads and writes
- Network protocol design and parsing
- Concurrent I/O and event loops
- In-memory data structures and key-value storage

## Reference

This project follows *Build Your Own Redis with C/C++* from build-your-own.org as a guided systems programming study.
