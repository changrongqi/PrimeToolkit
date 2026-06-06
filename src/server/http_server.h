#pragma once
// ============================================================
// http_server.h - Minimal multi-threaded HTTP server (Winsock2)
// Single responsibility: accept HTTP connections and dispatch routes
// ============================================================

#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>
#include <thread>
#include <atomic>
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
    // When open_browser is true, attempts to open localhost in default browser.
    bool start(int port = 8080, bool open_browser = true);

    // Stop server
    void stop();

    // Check if server is running
    bool is_running() const { return running_.load(); }

    int port() const { return port_; }

    // URL helpers (exposed for internal request parsing)
    void parse_query_params(const std::string& query_string,
                            std::unordered_map<std::string, std::string>& params);
    std::string url_decode(const std::string& str);

private:
    void accept_loop();
    void handle_client(uintptr_t client_socket);
    Response handle_request(const Request& req);
    std::string read_file(const std::string& filepath);
    std::string get_mime_type(const std::string& path);

    uintptr_t listen_socket_;
    int port_;
    std::atomic<bool> running_;
    std::thread accept_thread_;
    std::vector<std::thread> worker_threads_;

    std::unordered_map<std::string, RouteHandler> routes_;
    std::string static_dir_;
};

} // namespace HttpServer