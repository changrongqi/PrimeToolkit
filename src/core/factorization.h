#pragma once
// ============================================================
// factorization.h - Prime factorization
// Single responsibility: decompose numbers into prime factors
// ============================================================

#include <cstdint>
#include <vector>
#include <utility>

namespace PrimeCore {

// Returns vector of (prime_factor, exponent) pairs, sorted by factor.
// Uses trial division for small factors + Pollard's Rho (Brent variant)
// for large composite factors.
std::vector<std::pair<uint64_t, int>> factorize(uint64_t n);

} // namespace PrimeCore