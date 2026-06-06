// ============================================================
// factorization.cpp - Prime factorization
// Strategy: trial division for small factors + Pollard's Rho
//           (Brent variant) for large composite remnants.
// Supports full 128-bit range.
// ============================================================

#include "factorization.h"
#include "primality.h"
#include <algorithm>
#include <cstdlib>

namespace PrimeCore {

// ============================================================
// Internal helpers
// ============================================================

// 128-bit modular multiplication: (a * b) % mod.
// Uses native __uint128_t for intermediate product when values
// fit in 64-bit, falling back to binary decomposition for larger.
static inline int128_t mul_mod(int128_t a, int128_t b, int128_t mod) {
    if (a.value <= UINT64_MAX && b.value <= UINT64_MAX) {
        return (static_cast<native_u128>(a.lo()) * b.lo()) % mod.value;
    }
    native_u128 r = 0;
    a.value %= mod.value;
    b.value %= mod.value;
    while (b.value) {
        if (b.value & 1) {
            r += a.value;
            if (r >= mod.value) r -= mod.value;
        }
        a.value <<= 1;
        if (a.value >= mod.value) a.value -= mod.value;
        b.value >>= 1;
    }
    return r;
}

// Binary GCD for 128-bit integers
static inline int128_t binary_gcd(int128_t a, int128_t b) {
    if (a.value == 0) return b;
    if (b.value == 0) return a;

    int shift = 0;
    while (((a.value | b.value) & 1) == 0) {
        a.value >>= 1;
        b.value >>= 1;
        ++shift;
    }
    while ((a.value & 1) == 0) a.value >>= 1;
    do {
        while ((b.value & 1) == 0) b.value >>= 1;
        if (a.value > b.value) std::swap(a.value, b.value);
        b.value -= a.value;
    } while (b.value != 0);
    return int128_t(a.value << shift);
}

static inline int128_t abs_diff(int128_t a, int128_t b) {
    return a.value > b.value ? int128_t(a.value - b.value) : int128_t(b.value - a.value);
}

// ============================================================
// Pollard's Rho with Brent's cycle-finding improvement
// Returns a non-trivial factor, or n if none found.
// ============================================================
static int128_t pollard_rho_brent(int128_t n) {
    if ((n.value & 1) == 0) return int128_t(2);
    if (n.value % 3 == 0) return int128_t(3);

    // Seeds and offsets for multiple attempts
    const uint64_t seeds[]  = { 1, 2, 3, 5, 7, 11, 13, 17, 19, 23 };
    const uint64_t offsets[] = { 1, 3, 5, 7, 11, 13, 17, 19, 21, 25 };

    for (int attempt = 0; attempt < 5; ++attempt) {
        native_u128 c = offsets[attempt];
        native_u128 y = seeds[attempt];
        native_u128 x = y;
        native_u128 ys = y;

        native_u128 m = 128;
        native_u128 r = 1;
        native_u128 q = 1;
        native_u128 g = 1;

        while (g == 1) {
            x = y;
            for (native_u128 i = 0; i < r; ++i) {
                y = (mul_mod(int128_t(y), int128_t(y), n).value + c);
                if (y >= n.value) y -= n.value;
            }

            native_u128 k = 0;
            while (k < r && g == 1) {
                ys = y;
                native_u128 steps = (m < r - k) ? m : (r - k);
                for (native_u128 i = 0; i < steps; ++i) {
                    y = (mul_mod(int128_t(y), int128_t(y), n).value + c);
                    if (y >= n.value) y -= n.value;
                    q = mul_mod(int128_t(q),
                        int128_t(x > y ? x - y : y - x), n).value;
                }
                g = binary_gcd(int128_t(q), n).value;
                k += steps;
            }
            r <<= 1;
        }

        if (g == n.value) {
            // Backtrack to find actual factor
            do {
                ys = (mul_mod(int128_t(ys), int128_t(ys), n).value + c);
                if (ys >= n.value) ys -= n.value;
                g = binary_gcd(int128_t(x > ys ? x - ys : ys - x), n).value;
            } while (g == 1);

            if (g == n.value) continue;
        }

        if (g > 1 && g < n.value) return int128_t(g);
    }

    return n; // Failed to find factor
}

// ============================================================
// Trial division for small factors
// ============================================================
static void trial_divide(int128_t& n, std::vector<std::pair<int128_t, int>>& factors,
                         native_u128 limit) {
    // Handle 2
    if ((n.value & 1) == 0) {
        int exp = 0;
        while ((n.value & 1) == 0) {
            n.value >>= 1;
            ++exp;
        }
        factors.emplace_back(int128_t(2), exp);
    }

    // Handle 3
    while (n.value % 3 == 0) {
        int exp = 0;
        while (n.value % 3 == 0) {
            n.value /= 3;
            ++exp;
        }
        factors.emplace_back(int128_t(3), exp);
    }

    // Wheel for 5, 7, 11, 13, 17, 19, ...
    native_u128 i = 5;
    int wheel = 0;
    const int increments[] = { 2, 4, 2, 4, 6, 2, 6 };

    while (i <= limit && i * i <= n.value) {
        if (n.value % i == 0) {
            int exp = 0;
            while (n.value % i == 0) {
                n.value /= i;
                ++exp;
            }
            factors.emplace_back(int128_t(i), exp);
        }
        i += increments[wheel];
        wheel = (wheel + 1) % 7;
    }
}

// ============================================================
// Recursive factorization via Pollard's Rho
// ============================================================
static void factor_recursive(int128_t n, std::vector<std::pair<int128_t, int>>& factors) {
    if (n.value <= 1) return;
    if (is_prime(n)) {
        factors.emplace_back(n, 1);
        return;
    }

    int128_t factor = pollard_rho_brent(n);
    if (factor.value == n.value || factor.value <= 1) {
        // Pollard failed; treat as prime (extremely unlikely)
        factors.emplace_back(n, 1);
        return;
    }

    int exp = 0;
    while (n.value % factor.value == 0) {
        n.value /= factor.value;
        ++exp;
    }
    if (exp > 0) {
        factors.emplace_back(factor, exp);
    }

    if (n.value > 1) {
        factor_recursive(n, factors);
    }
}

// ============================================================
// Public API
// ============================================================

std::vector<std::pair<int128_t, int>> factorize(int128_t n) {
    std::vector<std::pair<int128_t, int>> result;
    if (n.value <= 1) return result;

    // Phase 1: Trial division for small factors (up to 1,000,000)
    trial_divide(n, result, 1000000ULL);

    // Phase 2: Pollard's Rho for remaining large composite
    if (n.value > 1) {
        if (is_prime(n)) {
            result.emplace_back(n, 1);
        } else {
            factor_recursive(n, result);
        }
    }

    // Sort by factor
    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) { return a.first.value < b.first.value; });

    // Merge duplicate factors from different phases
    if (result.size() > 1) {
        std::vector<std::pair<int128_t, int>> merged;
        merged.push_back(result[0]);
        for (size_t i = 1; i < result.size(); ++i) {
            if (result[i].first.value == merged.back().first.value) {
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