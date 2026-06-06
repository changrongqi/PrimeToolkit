// main.cpp  --  Application entry point
// Copyright (c) 2024 PrimeToolkit Project
//
// Single responsibility: configure and start the HTTP server.

#include "api.h"
#include "server/http_server.h"

#include <cstdio>
#include <string>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void print_banner() {
    printf("============================================\n");
    printf("  Prime Number Toolkit - C++ Core Server\n");
    printf("============================================\n\n");
}

std::string get_web_dir() {
    // First check REPLIT (or other cloud env)
    const char* web_env = std::getenv("WEB_DIR");
    if (web_env) {
        return std::string(web_env);
    }

    // Then check current directory
    return "./web";
}

int get_port() {
    const char* port_env = std::getenv("PORT");
    if (port_env) {
        return std::atoi(port_env);
    }
    return 8080;
}

bool is_replit() {
    const char* replit_env = std::getenv("REPL_ID");
    return replit_env != nullptr;
}

void register_routes(HttpServer::Server& server) {
    server.get("/api/is_prime",      handle_is_prime);
    server.get("/api/next_prime",    handle_next_prime);
    server.get("/api/prev_prime",    handle_prev_prime);
    server.get("/api/primes",        handle_primes);
    server.get("/api/primes_count",  handle_primes_count);
    server.get("/api/factorize",     handle_factorize);
    server.get("/api/nth_prime",     handle_nth_prime);
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    print_banner();

    std::string web_dir = get_web_dir();
    int port = get_port();
    bool open_browser = !is_replit(); // Don't open browser on Replit

    HttpServer::Server server;
    register_routes(server);
    server.serve_static(web_dir);

    printf("[*] Web directory: %s\n", web_dir.c_str());
    printf("[*] Starting server on http://0.0.0.0:%d\n", port);
    printf("[*] Press Ctrl+C to stop.\n\n");

    if (!server.start(port, open_browser)) {
        printf("[!] ERROR: Failed to start server. "
               "Port %d may be in use.\n", port);
        return 1;
    }

    if (is_replit()) {
        printf("[*] Server is running on Replit!\n");
        printf("[*] Your app is public at: [Replit Webview URL]\n\n");
    } else {
        printf("[*] Server is running.\n");
        printf("[*] If browser doesn't open, visit: "
               "http://localhost:%d\n\n", port);
    }

    printf("Press Ctrl+C to stop the server...\n");

    while (server.is_running()) {
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
    }

    return 0;
}