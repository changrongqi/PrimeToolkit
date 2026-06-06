// ============================================================
// main.cpp - Application entry point
// Single responsibility: wire up routes and start the server
// ============================================================

#include "core/prime_core.h"
#include "server/http_server.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <windows.h>

using namespace PrimeCore;

// ============================================================
// JSON helpers (no library dependency)
// ============================================================

static std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:   out += c;
        }
    }
    return out;
}

static std::string json_number(int128_t n) {
    return n.to_string();
}

static std::string json_bool(bool b) {
    return b ? "true" : "false";
}

// Build JSON array of [prime, exponent] pairs for factorization
static std::string factorization_json(const std::vector<std::pair<int128_t, int>>& factors) {
    std::string json = "[";
    for (size_t i = 0; i < factors.size(); ++i) {
        if (i > 0) json += ",";
        json += "[" + factors[i].first.to_string() + "," + std::to_string(factors[i].second) + "]";
    }
    json += "]";
    return json;
}

// Build JSON array of primes
static std::string primes_json(const std::vector<int128_t>& primes) {
    std::string json = "[";
    for (size_t i = 0; i < primes.size(); ++i) {
        if (i > 0) json += ",";
        json += primes[i].to_string();
    }
    json += "]";
    return json;
}

// ============================================================
// Query parameter helpers
// ============================================================

static bool get_query_int128(const HttpServer::Request& req, const std::string& key, int128_t& out) {
    auto it = req.query_params.find(key);
    if (it == req.query_params.end()) return false;
    try {
        out = int128_t::from_string(it->second);
        return true;
    } catch (...) {
        return false;
    }
}

static HttpServer::Response error_response(int code, const std::string& msg) {
    HttpServer::Response res;
    res.status_code = code;
    res.body = R"({"error":")" + json_escape(msg) + R"("})";
    return res;
}

static HttpServer::Response ok_response(const std::string& result_json) {
    HttpServer::Response res;
    res.status_code = 200;
    res.body = R"({"result":)" + result_json + "}";
    return res;
}

// ============================================================
// Route handlers
// ============================================================

// GET /api/is_prime?n=...
static HttpServer::Response handle_is_prime(const HttpServer::Request& req) {
    int128_t n;
    if (!get_query_int128(req, "n", n)) {
        return error_response(400, "Missing or invalid parameter 'n'");
    }
    return ok_response(json_bool(PrimeCore::is_prime(n)));
}

// GET /api/next_prime?n=...
static HttpServer::Response handle_next_prime(const HttpServer::Request& req) {
    int128_t n;
    if (!get_query_int128(req, "n", n)) {
        return error_response(400, "Missing or invalid parameter 'n'");
    }
    return ok_response(json_number(PrimeCore::next_prime(n)));
}

// GET /api/prev_prime?n=...
static HttpServer::Response handle_prev_prime(const HttpServer::Request& req) {
    int128_t n;
    if (!get_query_int128(req, "n", n)) {
        return error_response(400, "Missing or invalid parameter 'n'");
    }
    int128_t result = PrimeCore::prev_prime(n);
    if (result == 0) {
        return error_response(404, "No prime found below given number");
    }
    return ok_response(json_number(result));
}

// GET /api/primes?from=...&to=...
static HttpServer::Response handle_primes(const HttpServer::Request& req) {
    int128_t from, to;
    if (!get_query_int128(req, "from", from) || !get_query_int128(req, "to", to)) {
        return error_response(400, "Missing or invalid parameters 'from' and 'to'");
    }
    if (from > to) {
        return error_response(400, "'from' must be <= 'to'");
    }
    if (to.value - from.value > 10000000ULL) {
        return error_response(400, "Range too large (max 10,000,000)");
    }
    auto primes = PrimeCore::primes_in_range(from, to);
    return ok_response(primes_json(primes));
}

// GET /api/primes_count?from=...&to=...
static HttpServer::Response handle_primes_count(const HttpServer::Request& req) {
    int128_t from, to;
    if (!get_query_int128(req, "from", from) || !get_query_int128(req, "to", to)) {
        return error_response(400, "Missing or invalid parameters 'from' and 'to'");
    }
    if (from > to) {
        return error_response(400, "'from' must be <= 'to'");
    }
    return ok_response(std::to_string(PrimeCore::primes_count(from, to)));
}

// GET /api/factorize?n=...
static HttpServer::Response handle_factorize(const HttpServer::Request& req) {
    int128_t n;
    if (!get_query_int128(req, "n", n)) {
        return error_response(400, "Missing or invalid parameter 'n'");
    }
    if (n.value <= 1) {
        return error_response(400, "Number must be > 1");
    }
    auto factors = PrimeCore::factorize(n);
    return ok_response(factorization_json(factors));
}

// GET /api/nth_prime?n=...
static HttpServer::Response handle_nth_prime(const HttpServer::Request& req) {
    int128_t n;
    if (!get_query_int128(req, "n", n)) {
        return error_response(400, "Missing or invalid parameter 'n'");
    }
    if (n.value == 0 || n.value > 10000000ULL) {
        return error_response(400, "'n' must be between 1 and 10,000,000");
    }
    return ok_response(json_number(PrimeCore::nth_prime(n)));
}

// ============================================================
// Entry point
// ============================================================

int main() {
    // Set console to UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    printf("============================================\n");
    printf("  Prime Number Toolkit - C++ Core Server\n");
    printf("============================================\n\n");

    // Get the directory of the executable
    char exe_path[MAX_PATH];
    GetModuleFileNameA(nullptr, exe_path, MAX_PATH);
    std::string exe_dir(exe_path);
    size_t last_slash = exe_dir.rfind('\\');
    if (last_slash != std::string::npos) {
        exe_dir = exe_dir.substr(0, last_slash);
    }

    // Web directory relative to executable
    std::string web_dir = exe_dir + "\\web";

    HttpServer::Server server;

    // Register API routes
    server.get("/api/is_prime", handle_is_prime);
    server.get("/api/next_prime", handle_next_prime);
    server.get("/api/prev_prime", handle_prev_prime);
    server.get("/api/primes", handle_primes);
    server.get("/api/primes_count", handle_primes_count);
    server.get("/api/factorize", handle_factorize);
    server.get("/api/nth_prime", handle_nth_prime);

    // Serve static files from web directory
    server.serve_static(web_dir);

    printf("[*] Web directory: %s\n", web_dir.c_str());
    printf("[*] Starting server on http://localhost:8080\n");
    printf("[*] Press Ctrl+C or close this window to stop.\n\n");

    if (!server.start(8080, true)) {
        printf("[!] ERROR: Failed to start server. Port 8080 may be in use.\n");
        printf("[!] Press Enter to exit...\n");
        getchar();
        return 1;
    }

    printf("[*] Server is running. Opening browser...\n");
    printf("[*] If browser doesn't open, visit: http://localhost:8080\n\n");

    // Keep running until user presses Ctrl+C or closes window
    printf("Press Ctrl+C to stop the server...\n");

    // Simple wait loop
    while (server.is_running()) {
        Sleep(1000);
    }

    return 0;
}