#pragma once
// ============================================================
// factorization.h - Prime factorization
// Single responsibility: decompose numbers into prime factors
// Supports up to 128-bit integers via int128_t.
// ============================================================

#include "int128_t.h"
#include <vector>
#include <utility>

namespace PrimeCore {

// Returns vector of (prime_factor, exponent) pairs, sorted by factor.
// Uses trial division for small factors + Pollard's Rho (Brent variant)
// for large composite factors. Works for all n up to 2^128 - 1.
std::vector<std::pair<int128_t, int>> factorize(int128_t n);

} // namespace PrimeCore