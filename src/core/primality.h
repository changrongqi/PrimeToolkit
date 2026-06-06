// primality.h  --  Miller-Rabin deterministic primality test
// Copyright (c) 2024 PrimeToolkit Project
//
// Single responsibility: determine if a number is prime.
// Supports up to 128-bit integers via int128_t.

#pragma once

#include "core/int128_t.h"

namespace PrimeCore {

// Deterministic Miller-Rabin primality test.
// Returns true if n is definitely prime.
bool is_prime(int128_t n);

// Find next prime >= n.
int128_t next_prime(int128_t n);

// Find previous prime <= n (returns 0 if none found).
int128_t prev_prime(int128_t n);

}  // namespace PrimeCore
