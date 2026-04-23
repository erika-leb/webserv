# Webserv

A custom HTTP server written in C++, developed as part of the 42 curriculum.  
This project aims to reproduce the core behavior of a real web server by handling HTTP requests, managing connections, and serving dynamic and static content.

---

## 🚀 Features

- HTTP/1.1 request handling
- Support for multiple clients simultaneously
- Non-blocking I/O using `poll` / `epoll`
- Static file serving
- CGI execution (e.g. PHP, Python scripts)
- Configurable server via configuration file (inspired by nginx)
- Support for:
  - GET
  - POST
  - DELETE
- Error handling (custom error pages)
- File uploads

---

## 🧠 What I learned

This project focuses on system-level programming and network architecture:

- Socket programming (`socket`, `bind`, `listen`, `accept`)
- Multiplexing with `poll` / `epoll`
- HTTP protocol parsing and response construction
- Process management for CGI (`fork`, `execve`)
- Handling partial reads/writes and buffering
- Server scalability and performance considerations

---

## ⚙️ Architecture

The server follows an event-driven architecture:

1. **Socket setup**
   - Create and configure server sockets
   - Set non-blocking mode

2. **Event loop**
   - Monitor multiple file descriptors using `poll` or `epoll`
   - Accept new client connections
   - Read incoming requests
   - Write responses

3. **Request handling**
   - Parse HTTP request
   - Route to appropriate handler:
     - Static file
     - CGI execution
     - Error response

---

## 🛠️ Tech stack

- Language: C++ (C++98 standard)
- System calls: `socket`, `bind`, `listen`, `accept`, `poll` / `epoll`, `recv`, `send`
- Environment: Linux

---

## ▶️ Usage

make

./webserv [config_file]
