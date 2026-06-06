// sieve.h  --  Segmented sieve of Eratosthenes
// Copyright (c) 2024 PrimeToolkit Project
//
// Single responsibility: generate/count primes in ranges.
// Supports int128_t range bounds with practical memory limits.

#pragma once

#include <vector>

#include "core/int128_t.h"

namespace PrimeCore {

// Generate all primes in [from, to] (inclusive).
// Range width is limited to ~10^7 for memory reasons.
std::vector<int128_t> primes_in_range(int128_t from, int128_t to);

// Count primes in [from, to] using segmented sieve.
uint64_t primes_count(int128_t from, int128_t to);

// Get the nth prime (1-indexed: nth_prime(1) = 2).
// Uses prime number theorem approximation + sieve.
int128_t nth_prime(int128_t n);

}  // namespace PrimeCore
