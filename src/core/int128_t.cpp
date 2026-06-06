// int128_t.cpp - I/O implementation for int128_t
// Copyright (c) 2024 PrimeToolkit Project
//
// Single responsibility: decimal string <-> 128-bit conversion.

#include <algorithm>
#include <stdexcept>
#include <string>

#include "core/int128_t.h"

namespace PrimeCore {

// ---------- to_string: 128-bit integer -> decimal string ----------
// Uses repeated division by 10^9 (to minimize divisions).
// Complexity: O(log n) divisions, each division is native 128-bit.
std::string int128_t::to_string() const {
    if (value == 0) return "0";

    // Split into 9-digit chunks (10^9 fits in uint32_t for display)
    static const native_u128 BILLION = 1000000000ULL;
    char buf[40];  // 128 bits needs at most 39 decimal digits + null
    int pos = 39;
    buf[pos] = '\0';

    native_u128 n = value;
    while (n > 0) {
        uint32_t chunk = static_cast<uint32_t>(n % BILLION);
        n /= BILLION;
        // Write 9 digits (or fewer for the most significant chunk)
        int digits = 9;
        if (n == 0) {
            // Count digits in the last chunk
            if (chunk == 0) {
                digits = 1;
            } else {
                uint32_t tmp = chunk;
                digits = 0;
                while (tmp) {
                    tmp /= 10;
                    ++digits;
                }
            }
        }
        for (int i = 0; i < digits; ++i) {
            buf[--pos] = '0' + (chunk % 10);
            chunk /= 10;
        }
    }
    return std::string(buf + pos);
}

// ---------- from_string: decimal string -> 128-bit integer ----------
// Validates input and parses using base-10 multiplication.
// Throws std::invalid_argument on malformed or overflow input.
int128_t int128_t::from_string(const std::string& s) {
    if (s.empty()) {
        throw std::invalid_argument("int128_t::from_string: empty string");
    }

    size_t i = 0;
    // Skip leading whitespace
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
    if (i == s.size()) {
        throw std::invalid_argument("int128_t::from_string: no digits");
    }

    native_u128 result = 0;
    native_u128 max_before_mul = ~native_u128(0) / 10;
    int digit_count = 0;

    for (; i < s.size(); ++i) {
        char c = s[i];
        if (c < '0' || c > '9') {
            // Allow trailing whitespace
            if (c == ' ' || c == '\t') {
                while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
                if (i == s.size()) break;
            }
            throw std::invalid_argument(
                "int128_t::from_string: invalid character '"
                + std::string(1, c) + "'");
        }

        if (result > max_before_mul) {
            throw std::overflow_error("int128_t::from_string: overflow");
        }
        result *= 10;
        uint8_t digit = static_cast<uint8_t>(c - '0');
        if (result > ~native_u128(0) - digit) {
            throw std::overflow_error("int128_t::from_string: overflow");
        }
        result += digit;
        ++digit_count;
    }

    if (digit_count == 0) {
        throw std::invalid_argument("int128_t::from_string: no digits");
    }

    return int128_t(result);
}

}  // namespace PrimeCore
