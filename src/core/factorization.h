// factorization.h: Prime factorization
// Copyright (c) 2024 PrimeToolkit Project
//
// Single responsibility: decompose numbers into prime factors.
// Supports up to 128-bit integers via int128_t.

#pragma once

#include <utility>
#include <vector>

#include "core/int128_t.h"

namespace PrimeCore {

// Returns vector of (prime_factor, exponent) pairs, sorted by
// factor. Uses trial division for small factors + Pollard's Rho
// (Brent variant) for large composite factors.
// Works for all n up to 2^128 - 1.
std::vector<std::pair<int128_t, int>> factorize(int128_t n);

}  // namespace PrimeCore
