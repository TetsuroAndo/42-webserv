# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

- `make` - Build the webserv executable
- `make debug` - Build with debug flags (AddressSanitizer enabled)
- `make clean` - Remove object files
- `make fclean` - Remove object files and executable
- `make re` - Clean rebuild
- `make norm` - Run norminette on source files

## Running the Server

- `./webserv` - Start server on default port 8080
- `./webserv <port>` - Start server on specified port

## Architecture

This is a minimal HTTP web server implementation written in C++98, following 42 School standards.

### Core Components

- **Server class** (`inc/server.hpp`, `src/server.cpp`): Main server implementation using non-blocking sockets with poll() for I/O multiplexing
- **Main** (`src/main.cpp`): Entry point that initializes and runs the server

### Server Design

The server uses an event-driven architecture:
- Single-threaded with poll() for handling multiple connections
- Non-blocking sockets for both listening and client connections  
- Accepts connections on listening socket, adds client sockets to poll monitoring
- Handles basic HTTP GET requests with minimal response
- Currently closes connection after each request (HTTP/1.0 style)

### Configuration

- `config/default.conf` contains nginx-style configuration (not yet fully implemented)
- Server currently hardcoded to bind to 0.0.0.0:8080 by default

## Code Standards

- C++98 compliance required
- 42 School norminette standards
- Wall, Wextra, Werror compilation flags