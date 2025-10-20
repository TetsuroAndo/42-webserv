#ifndef PERFORMANCE_CONFIG_HPP
#define PERFORMANCE_CONFIG_HPP

// Buffer sizes for optimal performance
#define IO_BUFFER_SIZE 8192		  // Socket I/O buffer (increased from 4096)
#define CGI_BUFFER_SIZE 8192	  // CGI pipe I/O buffer
#define RESPONSE_RESERVE_SIZE 512 // Initial reserve for response strings

// Timeouts
#define CLIENT_TIMEOUT_SEC 60 // Client connection timeout
#define CGI_TIMEOUT_SEC 30	  // CGI execution timeout

// Limits
#define MAX_CONNECTIONS 1024	  // Maximum concurrent connections
#define MAX_HEADER_SIZE 8192	  // Maximum header size
#define MAX_REQUEST_BODY 10485760 // 10MB default max body size

// Poll/Epoll/Kqueue configuration
#define POLL_TIMEOUT_MS 1000 // Poll timeout in milliseconds
#define MAX_EVENTS 256		 // Maximum events per poll call

#endif // PERFORMANCE_CONFIG_HPP
