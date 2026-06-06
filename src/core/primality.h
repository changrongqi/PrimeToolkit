#pragma once
// ============================================================
// primality.h - Miller-Rabin deterministic primality test
// Single responsibility: determine if a number is prime
// ============================================================

#include <cstdint>

namespace PrimeCore {

// Deterministic Miller-Rabin for all 64-bit integers.
// Uses bases {2, 3, 5, 7, 11, 13, 17} - proven sufficient for n < 2^64.
bool is_prime(uint64_t n);

// Find next prime >= n
uint64_t next_prime(uint64_t n);

// Find previous prime <= n (returns 0 if none found)
uint64_t prev_prime(uint64_t n);

// Get the nth prime (1-indexed: nth_prime(1) = 2)
uint64_t nth_prime(uint64_t n);

} // namespace PrimeCore