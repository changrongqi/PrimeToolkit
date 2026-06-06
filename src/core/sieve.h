#pragma once
// ============================================================
// sieve.h - Segmented sieve of Eratosthenes
// Single responsibility: generate/count primes in ranges
// Supports int128_t range bounds with practical memory limits.
// ============================================================

#include "int128_t.h"
#include <vector>

namespace PrimeCore {

// Generate all primes in [from, to] (inclusive).
// Range width is limited to ~10^7 for memory reasons.
// Bounds can be up to 2^128-1.
std::vector<int128_t> primes_in_range(int128_t from, int128_t to);

// Count primes in [from, to] using segmented sieve.
uint64_t primes_count(int128_t from, int128_t to);

} // namespace PrimeCore