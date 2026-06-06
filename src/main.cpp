// main.cpp  --  Application entry point
// Copyright (c) 2024 PrimeToolkit Project
//
// Single responsibility: configure and start the HTTP server.

#include <windows.h>

#include <cstdio>
#include <string>

#include "api.h"
#include "server/http_server.h"

void print_banner() {
    printf("============================================\n");
    printf("  Prime Number Toolkit - C++ Core Server\n");
    printf("============================================\n\n");
}

std::string get_exe_dir() {
    char exe_path[MAX_PATH];
    GetModuleFileNameA(nullptr, exe_path, MAX_PATH);
    std::string dir(exe_path);
    size_t last_slash = dir.rfind('\\');
    if (last_slash != std::string::npos) {
        dir = dir.substr(0, last_slash);
    }
    return dir;
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
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    print_banner();

    std::string web_dir = get_exe_dir() + "\\web";

    HttpServer::Server server;
    register_routes(server);
    server.serve_static(web_dir);

    printf("[*] Web directory: %s\n", web_dir.c_str());
    printf("[*] Starting server on http://localhost:8080\n");
    printf("[*] Press Ctrl+C or close this window to stop.\n\n");

    if (!server.start(8080, true)) {
        printf("[!] ERROR: Failed to start server. "
               "Port 8080 may be in use.\n");
        printf("[!] Press Enter to exit...\n");
        getchar();
        return 1;
    }

    printf("[*] Server is running. Opening browser...\n");
    printf("[*] If browser doesn't open, visit: "
           "http://localhost:8080\n\n");
    printf("Press Ctrl+C to stop the server...\n");

    while (server.is_running()) {
        Sleep(1000);
    }

    return 0;
}