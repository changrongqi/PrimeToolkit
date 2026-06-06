// ============================================================
// primality.cpp - Miller-Rabin deterministic primality test
// ============================================================

#include "primality.h"
#include <cmath>
#include <algorithm>
#include <vector>

namespace PrimeCore {

// Modular multiplication: (a * b) % mod without overflow
// Uses binary decomposition (Russian peasant method), O(log b).
static inline uint64_t mul_mod(uint64_t a, uint64_t b, uint64_t mod) {
    if (mod <= 0xFFFFFFFFULL) {
        return (a * b) % mod;
    }
    uint64_t result = 0;
    a %= mod;
    while (b) {
        if (b & 1) {
            result = (result + a);
            if (result >= mod) result -= mod;
        }
        a = (a << 1);
        if (a >= mod) a -= mod;
        b >>= 1;
    }
    return result;
}

// Modular exponentiation: (base ^ exp) % mod
static inline uint64_t pow_mod(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    base %= mod;
    while (exp) {
        if (exp & 1) result = mul_mod(result, base, mod);
        base = mul_mod(base, base, mod);
        exp >>= 1;
    }
    return result;
}

// Deterministic Miller-Rabin for 64-bit integers.
// Bases proven sufficient for n < 2^64: {2, 325, 9375, 28178, 450775, 9780504, 1795265022}
static const uint64_t MR_BASES[] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};

static bool miller_rabin(uint64_t n) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if ((n & 1) == 0) return false;

    // Write n-1 = d * 2^s
    uint64_t d = n - 1;
    int s = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++s;
    }

    for (uint64_t a : MR_BASES) {
        if (a % n == 0) continue;

        uint64_t x = pow_mod(a, d, n);
        if (x == 1 || x == n - 1) continue;

        bool composite = true;
        for (int r = 0; r < s - 1; ++r) {
            x = mul_mod(x, x, n);
            if (x == n - 1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}

// Small prime check via trial division (fast path for small numbers)
static bool small_prime_check(uint64_t n) {
    if (n < 2) return false;
    if (n == 2 || n == 3 || n == 5 || n == 7) return true;
    if ((n & 1) == 0 || n % 3 == 0 || n % 5 == 0) return false;
    uint64_t limit = static_cast<uint64_t>(std::sqrt(static_cast<long double>(n))) + 1;
    for (uint64_t i = 7; i <= limit; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

bool is_prime(uint64_t n) {
    // Fast path for small numbers: trial division up to sqrt is faster
    // than Miller-Rabin setup for n < ~1,000,000
    if (n < 1000000ULL) {
        return small_prime_check(n);
    }
    return miller_rabin(n);
}

uint64_t next_prime(uint64_t n) {
    if (n <= 2) return 2;
    if (n == 3) return 3;
    // Start from first odd >= n
    uint64_t candidate = n | 1ULL;
    if (candidate == 1) candidate = 3;
    // Skip multiples of 3 using wheel
    while (true) {
        if (is_prime(candidate)) return candidate;
        candidate += 2;
        if (candidate % 3 == 0) candidate += 2;
    }
}

uint64_t prev_prime(uint64_t n) {
    if (n <= 2) return 0;
    if (n == 3) return 2;
    uint64_t candidate = (n - 1) | 1ULL; // Largest odd < n
    while (candidate >= 2) {
        if (is_prime(candidate)) return candidate;
        candidate -= 2;
        if (candidate % 3 == 0 && candidate > 3) candidate -= 2;
    }
    return 0;
}

uint64_t nth_prime(uint64_t n) {
    if (n == 0) return 0;
    if (n == 1) return 2;
    if (n == 2) return 3;

    // Approximate nth prime using prime number theorem:
    // p_n ~ n * (log n + log log n)
    double log_n = std::log(static_cast<double>(n));
    double log_log_n = std::log(log_n);
    uint64_t approx = static_cast<uint64_t>(n * (log_n + log_log_n)) + 2;

    // Generate primes up to approx using sieve, then count
    // Use a generous overestimate (20% margin)
    uint64_t limit = approx + (approx / 5) + 1000;

    // Use segmented sieve in a simple way for the estimate range
    std::vector<bool> is_composite(limit + 1, false);
    uint64_t count = 0;
    uint64_t last_prime = 0;

    for (uint64_t i = 2; i <= limit && count < n; ++i) {
        if (!is_composite[i]) {
            ++count;
            last_prime = i;
            if (count == n) return i;
            // Mark multiples
            if (i * i <= limit) {
                for (uint64_t j = i * i; j <= limit; j += i) {
                    is_composite[j] = true;
                }
            }
        }
    }

    return last_prime;
}

} // namespace PrimeCore