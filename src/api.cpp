// api.cpp  --  HTTP API route handlers implementation
// Copyright (c) 2024 PrimeToolkit Project
//
// Single responsibility: implement JSON helpers, query parsing,
// and route handlers for the prime toolkit API.

#include <string>
#include <utility>
#include <vector>

#include "api.h"
#include "core/prime_core.h"

using PrimeCore::int128_t;

// ============================================================
// JSON helpers
// ============================================================

std::string json_number(int128_t n) {
    return "\"" + n.to_string() + "\"";
}

std::string json_bool(bool b) {
    return b ? "true" : "false";
}

std::string factorization_json(
    const std::vector<std::pair<int128_t, int>>& factors) {
    std::string json = "[";
    for (size_t i = 0; i < factors.size(); ++i) {
        if (i > 0) json += ",";
        json += "[\"" + factors[i].first.to_string() + "\"," +
                std::to_string(factors[i].second) + "]";
    }
    json += "]";
    return json;
}

std::string primes_json(const std::vector<int128_t>& primes) {
    std::string json = "[";
    for (size_t i = 0; i < primes.size(); ++i) {
        if (i > 0) json += ",";
        json += "\"" + primes[i].to_string() + "\"";
    }
    json += "]";
    return json;
}

// ============================================================
// Query parameter helpers
// ============================================================

bool get_query_int128(const HttpServer::Request& req,
                      const std::string& key,
                      int128_t& out) {
    auto it = req.query_params.find(key);
    if (it == req.query_params.end()) return false;
    try {
        out = int128_t::from_string(it->second);
        return true;
    } catch (...) {
        return false;
    }
}

HttpServer::Response validate_range(const HttpServer::Request& req) {
    int128_t from, to;
    if (!get_query_int128(req, "from", from) ||
        !get_query_int128(req, "to", to)) {
        return error_response(400,
            "Missing or invalid parameters 'from' and 'to'");
    }
    if (from > to) {
        return error_response(400, "'from' must be <= 'to'");
    }
    return {};
}

HttpServer::Response error_response(int code, const std::string& msg) {
    HttpServer::Response res;
    res.status_code = code;
    res.body = "{\"error\":\"" + msg + "\"}";
    return res;
}

HttpServer::Response ok_response(const std::string& result_json) {
    HttpServer::Response res;
    res.status_code = 200;
    res.body = R"({"result":)" + result_json + "}";
    return res;
}

// ============================================================
// Route handlers
// ============================================================

HttpServer::Response handle_is_prime(const HttpServer::Request& req) {
    int128_t n;
    if (!get_query_int128(req, "n", n)) {
        return error_response(400, "Missing or invalid parameter 'n'");
    }
    return ok_response(json_bool(PrimeCore::is_prime(n)));
}

HttpServer::Response handle_next_prime(const HttpServer::Request& req) {
    int128_t n;
    if (!get_query_int128(req, "n", n)) {
        return error_response(400, "Missing or invalid parameter 'n'");
    }
    return ok_response(json_number(PrimeCore::next_prime(n)));
}

HttpServer::Response handle_prev_prime(const HttpServer::Request& req) {
    int128_t n;
    if (!get_query_int128(req, "n", n)) {
        return error_response(400, "Missing or invalid parameter 'n'");
    }
    int128_t result = PrimeCore::prev_prime(n);
    if (result == 0) {
        return error_response(404,
            "No prime found below given number");
    }
    return ok_response(json_number(result));
}

HttpServer::Response handle_primes(const HttpServer::Request& req) {
    auto err = validate_range(req);
    if (err.status_code != 0) return err;

    int128_t from, to;
    get_query_int128(req, "from", from);
    get_query_int128(req, "to", to);

    if (to.value - from.value > 10000000ULL) {
        return error_response(400, "Range too large (max 10,000,000)");
    }
    auto primes = PrimeCore::primes_in_range(from, to);
    return ok_response(primes_json(primes));
}

HttpServer::Response handle_primes_count(const HttpServer::Request& req) {
    auto err = validate_range(req);
    if (err.status_code != 0) return err;

    int128_t from, to;
    get_query_int128(req, "from", from);
    get_query_int128(req, "to", to);

    return ok_response(
        std::to_string(PrimeCore::primes_count(from, to)));
}

HttpServer::Response handle_factorize(const HttpServer::Request& req) {
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

HttpServer::Response handle_nth_prime(const HttpServer::Request& req) {
    int128_t n;
    if (!get_query_int128(req, "n", n)) {
        return error_response(400, "Missing or invalid parameter 'n'");
    }
    if (n.value == 0 || n.value > 10000000ULL) {
        return error_response(400,
            "'n' must be between 1 and 10,000,000");
    }
    return ok_response(json_number(PrimeCore::nth_prime(n)));
}