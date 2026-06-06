// http_server.h  --  Minimal multi-threaded HTTP server
// Copyright (c) 2024 PrimeToolkit Project
//
// Single responsibility: accept HTTP connections and dispatch
// routes via a fixed-size thread pool.
// Cross-platform: Windows (Winsock2) and POSIX (Linux/macOS).

#pragma once

#include "server/socket.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace HttpServer {

// HTTP request structure
struct Request {
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> query_params;
    std::string body;
};

// HTTP response structure
struct Response {
    int status_code = 200;
    std::string content_type = "application/json";
    std::string body;
    std::unordered_map<std::string, std::string> headers;
};

// Route handler: takes Request, returns Response
using RouteHandler = std::function<Response(const Request&)>;

class Server {
 public:
    Server();
    ~Server();

    // Non-copyable, non-movable
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    // Register a GET route
    void get(const std::string& path, RouteHandler handler);

    // Serve static files from directory (relative to executable)
    void serve_static(const std::string& dir);

    // Start server on given port. Returns true on success.
    // When open_browser is true, opens localhost in browser.
    bool start(int port = 8080, bool open_browser = true);

    // Stop server
    void stop();

    // Check if server is running
    bool is_running() const { return running_.load(); }

    int port() const { return port_; }

 private:
    void accept_loop();
    void worker_loop();
    void handle_client(SocketType client_socket);
    Response handle_request(const Request& req);
    std::string read_file(const std::string& filepath);
    std::string get_mime_type(const std::string& path);

    SocketType listen_socket_;
    int port_;
    std::atomic<bool> running_;

    // Accept thread
    std::thread accept_thread_;

    // Thread pool
    std::vector<std::thread> worker_threads_;
    std::queue<SocketType> client_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    std::unordered_map<std::string, RouteHandler> routes_;
    std::string static_dir_;
};

}  // namespace HttpServer
