// http_server.cpp  --  HTTP server implementation
// Copyright (c) 2024 PrimeToolkit Project
//
// Single responsibility: handle HTTP connections and serve
// static files alongside registered API routes.
// Uses 4KB buffered socket I/O and a fixed-size thread pool.
// Cross-platform: Windows (Winsock2) and POSIX (Linux/macOS).

#include "server/http_server.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#ifdef PRIME_SOCKET_WINDOWS
    #include <shellapi.h>
#else
    #include <spawn.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

namespace HttpServer {

// ---------- Utility ----------

static const char* get_status_message(int code) {
    switch (code) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default:  return "Unknown";
    }
}

Server::Server()
    : listen_socket_(PRIME_INVALID_SOCKET)
    , port_(0)
    , running_(false) {
    PrimeSocket::init();
}

Server::~Server() {
    stop();
    PrimeSocket::cleanup();
}

void Server::get(const std::string& path, RouteHandler handler) {
    routes_[path] = std::move(handler);
}

void Server::serve_static(const std::string& dir) {
    static_dir_ = dir;
}

// ---------- URL decoding ----------

static std::string url_decode_impl(const std::string& str) {
    std::string result;
    result.reserve(str.size());
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%' && i + 2 < str.size()) {
            auto hex_to_int = [](char c) -> int {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                return -1;
            };
            int h1 = hex_to_int(str[i + 1]);
            int h2 = hex_to_int(str[i + 2]);
            if (h1 >= 0 && h2 >= 0) {
                result += static_cast<char>((h1 << 4) | h2);
                i += 2;
                continue;
            }
        }
        result += (str[i] == '+') ? ' ' : str[i];
    }
    return result;
}

// Static helper for request parsing
static void parse_query_params_static(
    const std::string& query_string,
    std::unordered_map<std::string, std::string>& params,
    const std::function<std::string(const std::string&)>& decoder) {
    if (query_string.empty()) return;
    size_t pos = 0;
    while (pos < query_string.size()) {
        size_t eq = query_string.find('=', pos);
        size_t amp = query_string.find('&', pos);
        if (amp == std::string::npos) amp = query_string.size();
        if (eq != std::string::npos && eq < amp) {
            params[decoder(query_string.substr(pos, eq - pos))] =
                decoder(query_string.substr(eq + 1, amp - eq - 1));
        }
        pos = amp + 1;
    }
}

// ---------- File serving ----------

std::string Server::get_mime_type(const std::string& path) {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos) return "application/octet-stream";

    std::string ext = path.substr(dot);
    if (ext == ".html" || ext == ".htm")
        return "text/html; charset=utf-8";
    if (ext == ".css")
        return "text/css; charset=utf-8";
    if (ext == ".js")
        return "application/javascript; charset=utf-8";
    if (ext == ".json") return "application/json";
    if (ext == ".png")  return "image/png";
    if (ext == ".svg")  return "image/svg+xml";
    if (ext == ".ico")  return "image/x-icon";
    return "application/octet-stream";
}

std::string Server::read_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return "";
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string buffer(static_cast<size_t>(size), '\0');
    if (file.read(buffer.data(), size)) {
        return buffer;
    }
    return "";
}

// ============================================================
// Buffered socket reader (4KB buffer)
// ============================================================
class SocketReader {
 public:
    explicit SocketReader(SocketType sock) : sock_(sock) {}

    // Read one line (strips CR/LF). Returns empty string on EOF.
    std::string read_line() {
        std::string line;
        line.reserve(256);
        while (true) {
            int c = read_char();
            if (c < 0) return line;
            if (c == '\r') {
                int next = read_char();
                if (next >= 0 && next != '\n') {
                    --pos_;
                }
                break;
            }
            if (c == '\n') break;
            line += static_cast<char>(c);
        }
        return line;
    }

    // Read exactly 'count' bytes into buffer. Returns actual bytes read.
    int read_bytes(char* out, int count) {
        int total = 0;
        while (total < count) {
            int c = read_char();
            if (c < 0) break;
            out[total++] = static_cast<char>(c);
        }
        return total;
    }

 private:
    bool refill() {
        pos_ = 0;
        int n = recv(sock_, buf_, sizeof(buf_), 0);
        if (n <= 0) return false;
        end_ = n;
        return true;
    }

    int read_char() {
        if (pos_ >= end_ && !refill()) return -1;
        return static_cast<unsigned char>(buf_[pos_++]);
    }

    SocketType sock_;
    char buf_[4096];
    int pos_ = 0;
    int end_ = 0;
};

// ---------- Request parsing ----------

static Request parse_http_request(SocketType sock) {
    Request req;
    SocketReader reader(sock);

    std::string request_line = reader.read_line();
    if (request_line.empty()) return req;

    std::istringstream line_stream(request_line);
    line_stream >> req.method >> req.path;

    size_t qmark = req.path.find('?');
    std::string query_string;
    if (qmark != std::string::npos) {
        query_string = req.path.substr(qmark + 1);
        req.path = req.path.substr(0, qmark);
    }

    if (!query_string.empty()) {
        parse_query_params_static(
            query_string, req.query_params, url_decode_impl);
    }

    int content_length = 0;
    while (true) {
        std::string header = reader.read_line();
        if (header.empty()) break;

        auto cl_pos = header.find("Content-Length:");
        if (cl_pos != std::string::npos) {
            std::string cl_val = header.substr(cl_pos + 15);
            size_t start = cl_val.find_first_not_of(" \t");
            if (start != std::string::npos) {
                content_length = std::stoi(cl_val.substr(start));
            }
        }
    }

    if (content_length > 0) {
        req.body.resize(content_length);
        int total = reader.read_bytes(req.body.data(), content_length);
        req.body.resize(total);
    }

    return req;
}

// ---------- Route dispatching ----------

Response Server::handle_request(const Request& req) {
    if (req.method != "GET") {
        Response res;
        res.status_code = 405;
        res.body = R"({"error": "Method not allowed"})";
        return res;
    }

    auto it = routes_.find(req.path);
    if (it != routes_.end()) {
        return it->second(req);
    }

    if (!static_dir_.empty()) {
        std::string filepath = static_dir_ + "/" +
            (req.path == "/" ? "index.html" : req.path);
        if (filepath.find("..") != std::string::npos) {
            Response res;
            res.status_code = 404;
            res.body = R"({"error": "Not found"})";
            return res;
        }
        std::string content = read_file(filepath);
        if (!content.empty()) {
            Response res;
            res.status_code = 200;
            res.content_type = get_mime_type(filepath);
            res.body = std::move(content);
            return res;
        }
    }

    Response res;
    res.status_code = 404;
    res.body = R"({"error": "Not found"})";
    return res;
}

// ---------- Client handling ----------

void Server::handle_client(SocketType sock) {
    PrimeSocket::set_timeout(sock, 10000);

    Request req = parse_http_request(sock);
    Response res = handle_request(req);

    std::string http_response;
    http_response.reserve(256 + res.body.size());

    const char* status_msg = get_status_message(res.status_code);

    http_response += "HTTP/1.1 " +
        std::to_string(res.status_code) + " " + status_msg + "\r\n";
    http_response += "Content-Type: " + res.content_type + "\r\n";
    http_response += "Content-Length: " +
        std::to_string(res.body.size()) + "\r\n";
    http_response += "Connection: close\r\n";
    http_response += "Access-Control-Allow-Origin: *\r\n";

    for (const auto& [key, val] : res.headers) {
        http_response += key + ": " + val + "\r\n";
    }
    http_response += "\r\n";
    http_response += res.body;

    send(sock, http_response.data(),
         static_cast<int>(http_response.size()), 0);
    PrimeSocket::close_socket(sock);
}

// ============================================================
// Thread pool
// ============================================================

void Server::worker_loop() {
    while (true) {
        SocketType client;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this]() {
                return !client_queue_.empty() || !running_.load();
            });
            if (!running_.load() && client_queue_.empty()) return;
            client = client_queue_.front();
            client_queue_.pop();
        }
        handle_client(client);
    }
}

// ---------- Accept loop ----------

void Server::accept_loop() {
    while (running_.load()) {
        SocketType client = accept(listen_socket_, nullptr, nullptr);
        if (client == PRIME_INVALID_SOCKET) {
            if (running_.load()) {
                PrimeSocket::sleep_ms(50);
            }
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            client_queue_.push(client);
        }
        queue_cv_.notify_one();
    }
}

// ---------- Open browser (cross-platform) ----------

static void open_browser(const std::string& url) {
#ifdef PRIME_SOCKET_WINDOWS
    ShellExecuteA(nullptr, "open", url.c_str(),
                  nullptr, nullptr, SW_SHOWNORMAL);
#else
    pid_t pid = fork();
    if (pid == 0) {
        execlp("xdg-open", "xdg-open", url.c_str(),
               reinterpret_cast<char*>(nullptr));
        execlp("open", "open", url.c_str(),
               reinterpret_cast<char*>(nullptr));
        _exit(1);
    }
#endif
}

// ---------- Start / Stop ----------

bool Server::start(int port, bool open_browser_flag) {
    port_ = port;

    listen_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket_ == PRIME_INVALID_SOCKET) {
        return false;
    }

    int reuse = 1;
#ifdef PRIME_SOCKET_WINDOWS
    setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&reuse), sizeof(reuse));
#else
    setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR,
               &reuse, sizeof(reuse));
#endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<u_short>(port));

    if (bind(listen_socket_,
             reinterpret_cast<sockaddr*>(&addr),
             sizeof(addr)) == PRIME_SOCKET_ERROR) {
        PrimeSocket::close_socket(listen_socket_);
        listen_socket_ = PRIME_INVALID_SOCKET;
        return false;
    }

    if (listen(listen_socket_, SOMAXCONN) == PRIME_SOCKET_ERROR) {
        PrimeSocket::close_socket(listen_socket_);
        listen_socket_ = PRIME_INVALID_SOCKET;
        return false;
    }

    running_.store(true);

    int num_workers = static_cast<int>(std::thread::hardware_concurrency());
    if (num_workers < 2) num_workers = 2;
    if (num_workers > 8) num_workers = 8;
    for (int i = 0; i < num_workers; ++i) {
        worker_threads_.emplace_back(&Server::worker_loop, this);
    }

    accept_thread_ = std::thread(&Server::accept_loop, this);

    if (open_browser_flag) {
        std::string url =
            "http://localhost:" + std::to_string(port) + "/";
        open_browser(url);
    }

    return true;
}

void Server::stop() {
    running_.store(false);

    queue_cv_.notify_all();

    if (listen_socket_ != PRIME_INVALID_SOCKET) {
        PrimeSocket::close_socket(listen_socket_);
        listen_socket_ = PRIME_INVALID_SOCKET;
    }

    for (auto& t : worker_threads_) {
        if (t.joinable()) t.join();
    }
    worker_threads_.clear();

    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }
}

}  // namespace HttpServer
