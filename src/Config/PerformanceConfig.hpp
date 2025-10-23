#ifndef PERFORMANCE_CONFIG_HPP
#define PERFORMANCE_CONFIG_HPP

// Buffer sizes for optimal performance
#define IO_BUFFER_SIZE 8192		  // Socket I/O buffer (increased from 4096)
#define CGI_BUFFER_SIZE 8192	  // CGI pipe I/O buffer
#define RESPONSE_RESERVE_SIZE 512 // Initial reserve for response strings

// Poll/Epoll/Kqueue configuration
#define POLL_TIMEOUT_MS 1000 // Poll timeout in milliseconds

#endif // PERFORMANCE_CONFIG_HPP
