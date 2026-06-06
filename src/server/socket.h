// socket.h  --  Cross-platform socket abstraction
// Copyright (c) 2024 PrimeToolkit Project
//
// Single responsibility: abstract Winsock2 (Windows) and
// POSIX socket (Linux/macOS) into a unified API.

#pragma once

#ifdef _WIN32
    #define PRIME_SOCKET_WINDOWS 1
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using SocketType = SOCKET;
    #define PRIME_INVALID_SOCKET INVALID_SOCKET
    #define PRIME_SOCKET_ERROR SOCKET_ERROR
#else
    #define PRIME_SOCKET_POSIX 1
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <netdb.h>
    #include <errno.h>
    using SocketType = int;
    #define PRIME_INVALID_SOCKET (-1)
    #define PRIME_SOCKET_ERROR (-1)
    #define closesocket close
#endif

namespace PrimeSocket {

// Initialize socket library (call once at startup)
inline bool init() {
#ifdef PRIME_SOCKET_WINDOWS
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0;
#else
    return true;
#endif
}

// Cleanup socket library (call once at shutdown)
inline void cleanup() {
#ifdef PRIME_SOCKET_WINDOWS
    WSACleanup();
#endif
}

// Set socket to non-blocking mode
inline bool set_nonblocking(SocketType sock) {
#ifdef PRIME_SOCKET_WINDOWS
    u_long mode = 1;
    return ioctlsocket(sock, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(sock, F_GETFL, 0);
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK) != -1;
#endif
}

// Set socket timeout
inline bool set_timeout(SocketType sock, int ms) {
#ifdef PRIME_SOCKET_WINDOWS
    DWORD timeout = static_cast<DWORD>(ms);
    return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
                      reinterpret_cast<const char*>(&timeout),
                      sizeof(timeout)) == 0;
#else
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
                      &tv, sizeof(tv)) == 0;
#endif
}

// Sleep for milliseconds (cross-platform)
inline void sleep_ms(int ms) {
#ifdef PRIME_SOCKET_WINDOWS
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

// Get last socket error code
inline int get_error() {
#ifdef PRIME_SOCKET_WINDOWS
    return WSAGetLastError();
#else
    return errno;
#endif
}

// Close socket safely
inline void close_socket(SocketType sock) {
    if (sock != PRIME_INVALID_SOCKET) {
        closesocket(sock);
    }
}

}  // namespace PrimeSocket
