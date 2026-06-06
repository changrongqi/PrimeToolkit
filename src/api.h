// api.h  --  HTTP API route handlers
// Copyright (c) 2024 PrimeToolkit Project
//
// Single responsibility: JSON helpers, query parsing, and
// route handlers for the prime toolkit API.

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "core/prime_core.h"
#include "server/http_server.h"

// ============================================================
// JSON helpers (no library dependency)
// ============================================================

// Return as quoted string to prevent JS BigInt precision loss
std::string json_number(PrimeCore::int128_t n);

std::string json_bool(bool b);

// Build JSON array of [prime, exponent] pairs
std::string factorization_json(
    const std::vector<std::pair<PrimeCore::int128_t, int>>& factors);

// Build JSON array of primes (as quoted strings)
std::string primes_json(const std::vector<PrimeCore::int128_t>& primes);

// ============================================================
// Query parameter helpers
// ============================================================

// Parse a 128-bit integer query parameter
bool get_query_int128(const HttpServer::Request& req,
                      const std::string& key,
                      PrimeCore::int128_t& out);

// Unified boundary validation: returns error if from > to
HttpServer::Response validate_range(
    const HttpServer::Request& req);

HttpServer::Response error_response(int code,
                                    const std::string& msg);

HttpServer::Response ok_response(const std::string& result_json);

// ============================================================
// Route handlers
// ============================================================

HttpServer::Response handle_is_prime(const HttpServer::Request& req);
HttpServer::Response handle_next_prime(const HttpServer::Request& req);
HttpServer::Response handle_prev_prime(const HttpServer::Request& req);
HttpServer::Response handle_primes(const HttpServer::Request& req);
HttpServer::Response handle_primes_count(const HttpServer::Request& req);
HttpServer::Response handle_factorize(const HttpServer::Request& req);
HttpServer::Response handle_nth_prime(const HttpServer::Request& req);