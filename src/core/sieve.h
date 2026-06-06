#pragma once
// ============================================================
// sieve.h - Segmented sieve of Eratosthenes
// Single responsibility: generate/count primes in ranges
// ============================================================

#include <cstdint>
#include <vector>

namespace PrimeCore {

// Generate all primes in [from, to] (inclusive).
// Uses segmented sieve with wheel-7 optimization for large ranges.
std::vector<uint64_t> primes_in_range(uint64_t from, uint64_t to);

// Count primes in [from, to] using optimized Meissel-Lehmer for
// very large ranges, falling back to segmented sieve for smaller ones.
uint64_t primes_count(uint64_t from, uint64_t to);

} // namespace PrimeCore