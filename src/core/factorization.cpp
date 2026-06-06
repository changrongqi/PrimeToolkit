// ============================================================
// factorization.cpp - Prime factorization
// Strategy: trial division for small factors + Pollard's Rho
//           (Brent variant) for large composite remnants
// ============================================================

#include "factorization.h"
#include "primality.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cstdlib>

namespace PrimeCore {

// ---------- Internal helpers ----------

// 64-bit modular multiplication without overflow
static inline uint64_t mul_mod(uint64_t a, uint64_t b, uint64_t mod) {
    if (mod <= 0xFFFFFFFFULL) {
        return (a * b) % mod;
    }
    uint64_t r = 0;
    a %= mod;
    b %= mod;
    while (b) {
        if (b & 1) {
            r += a;
            if (r >= mod) r -= mod;
        }
        a <<= 1;
        if (a >= mod) a -= mod;
        b >>= 1;
    }
    return r;
}

// Binary GCD - faster than std::gcd for large numbers
static inline uint64_t binary_gcd(uint64_t a, uint64_t b) {
    if (a == 0) return b;
    if (b == 0) return a;

    int shift = 0;
    while (((a | b) & 1) == 0) {
        a >>= 1;
        b >>= 1;
        ++shift;
    }
    while ((a & 1) == 0) a >>= 1;
    do {
        while ((b & 1) == 0) b >>= 1;
        if (a > b) std::swap(a, b);
        b -= a;
    } while (b != 0);
    return a << shift;
}

static inline uint64_t abs_diff(uint64_t a, uint64_t b) {
    return a > b ? a - b : b - a;
}

// Pollard's Rho with Brent's cycle-finding improvement.
// Returns a non-trivial factor, or n if none found.
static uint64_t pollard_rho_brent(uint64_t n) {
    if ((n & 1) == 0) return 2;
    if (n % 3 == 0) return 3;

    // Use random seeds from different ranges
    const uint64_t seeds[] = { 1, 2, 3, 5, 7, 11, 13, 17, 19, 23 };
    const uint64_t offsets[] = { 1, 3, 5, 7, 11, 13, 17, 19, 21, 25 };

    for (int attempt = 0; attempt < 5; ++attempt) {
        uint64_t c = offsets[attempt];
        uint64_t y = seeds[attempt];
        uint64_t x = y;
        uint64_t ys = y;

        uint64_t m = 128;
        uint64_t r = 1;
        uint64_t q = 1;
        uint64_t g = 1;

        while (g == 1) {
            x = y;
            for (uint64_t i = 0; i < r; ++i) {
                y = (mul_mod(y, y, n) + c);
                if (y >= n) y -= n;
            }

            uint64_t k = 0;
            while (k < r && g == 1) {
                ys = y;
                uint64_t steps = std::min(m, r - k);
                for (uint64_t i = 0; i < steps; ++i) {
                    y = (mul_mod(y, y, n) + c);
                    if (y >= n) y -= n;
                    q = mul_mod(q, abs_diff(x, y), n);
                }
                g = binary_gcd(q, n);
                k += steps;
            }
            r <<= 1;
        }

        if (g == n) {
            // Backtrack to find actual factor
            do {
                ys = (mul_mod(ys, ys, n) + c);
                if (ys >= n) ys -= n;
                g = binary_gcd(abs_diff(x, ys), n);
            } while (g == 1);

            if (g == n) continue; // Try next seed
        }

        if (g > 1 && g < n) return g;
    }

    return n; // Failed to find factor
}

// Trial division up to 'limit', extracting all small factors.
static void trial_divide(uint64_t& n, std::vector<std::pair<uint64_t, int>>& factors,
                         uint64_t limit) {
    // Handle 2 separately for efficiency
    if ((n & 1) == 0) {
        int exp = 0;
        while ((n & 1) == 0) {
            n >>= 1;
            ++exp;
        }
        factors.emplace_back(2ULL, exp);
    }

    // Handle 3
    while (n % 3 == 0) {
        int exp = 0;
        while (n % 3 == 0) {
            n /= 3;
            ++exp;
        }
        factors.emplace_back(3ULL, exp);
    }

    // Wheel for 5, 7, 11, 13, ...
    uint64_t i = 5;
    int wheel = 0;
    // Pattern: 2, 4, 2, 4, 6, 2, 6, ... (skip multiples of 2 and 3)
    const int increments[] = { 2, 4, 2, 4, 6, 2, 6 };

    while (i <= limit && i * i <= n) {
        if (n % i == 0) {
            int exp = 0;
            while (n % i == 0) {
                n /= i;
                ++exp;
            }
            factors.emplace_back(i, exp);
        }
        i += increments[wheel];
        wheel = (wheel + 1) % 7;
    }
}

// Recursively factor a composite number using Pollard's Rho
static void factor_recursive(uint64_t n, std::vector<std::pair<uint64_t, int>>& factors) {
    if (n <= 1) return;
    if (is_prime(n)) {
        factors.emplace_back(n, 1);
        return;
    }

    uint64_t factor = pollard_rho_brent(n);
    if (factor == n || factor <= 1) {
        // Pollard failed; treat as prime (extremely unlikely for n < 2^64)
        factors.emplace_back(n, 1);
        return;
    }

    // Count multiplicity of the found factor
    int exp = 0;
    while (n % factor == 0) {
        n /= factor;
        ++exp;
    }
    if (exp > 0) {
        factors.emplace_back(factor, exp);
    }

    // Recurse on remaining part
    if (n > 1) {
        factor_recursive(n, factors);
    }
}

// ---------- Public API ----------

std::vector<std::pair<uint64_t, int>> factorize(uint64_t n) {
    std::vector<std::pair<uint64_t, int>> result;
    if (n <= 1) return result;

    // Phase 1: Trial division for small factors (up to 1,000,000)
    trial_divide(n, result, 1000000ULL);

    // Phase 2: Pollard's Rho for remaining large composite
    if (n > 1) {
        if (is_prime(n)) {
            result.emplace_back(n, 1);
        } else {
            factor_recursive(n, result);
        }
    }

    // Sort by factor (trial division already sorted; Pollard insertion may not be)
    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    // Merge duplicate factors from different phases
    if (result.size() > 1) {
        std::vector<std::pair<uint64_t, int>> merged;
        merged.push_back(result[0]);
        for (size_t i = 1; i < result.size(); ++i) {
            if (result[i].first == merged.back().first) {
                merged.back().second += result[i].second;
            } else {
                merged.push_back(result[i]);
            }
        }
        return merged;
    }

    return result;
}

} // namespace PrimeCore