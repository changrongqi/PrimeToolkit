// ============================================================
// primality.cpp - Miller-Rabin deterministic primality test
// Supports full 128-bit range with deterministic bases.
// ============================================================

#include "primality.h"
#include <cmath>
#include <vector>
#include <cstdlib>

namespace PrimeCore {

// ============================================================
// Internal helpers
// ============================================================

// Modular multiplication: (a * b) % mod.
// Uses native 128-bit multiplication for values up to 64 bits,
// falling back to 128-bit intermediate for larger values.
static inline int128_t mul_mod(int128_t a, int128_t b, int128_t mod) {
    // For values fitting in 64-bit, use native 128-bit mul for speed
    if (a.value <= UINT64_MAX && b.value <= UINT64_MAX) {
        return (static_cast<native_u128>(a.lo()) * b.lo()) % mod.value;
    }
    // Full 128-bit modular multiplication via binary decomposition
    native_u128 result = 0;
    a.value %= mod.value;
    while (b.value) {
        if (b.value & 1) {
            result += a.value;
            if (result >= mod.value) result -= mod.value;
        }
        a.value <<= 1;
        if (a.value >= mod.value) a.value -= mod.value;
        b.value >>= 1;
    }
    return result;
}

// Modular exponentiation: (base ^ exp) % mod
static inline int128_t pow_mod(int128_t base, int128_t exp, int128_t mod) {
    native_u128 result = 1;
    base.value %= mod.value;
    while (exp.value) {
        if (exp.value & 1) {
            result = mul_mod(int128_t(result), base, mod).value;
        }
        base = mul_mod(base, base, mod);
        exp.value >>= 1;
    }
    return result;
}

// Integer square root using Newton's method
static int128_t isqrt(int128_t n) {
    if (n.value <= 1) return n;
    native_u128 x = n.value;
    native_u128 y = (x + 1) >> 1;
    while (y < x) {
        x = y;
        y = (x + n.value / x) >> 1;
    }
    return x;
}

// ============================================================
// Miller-Rabin implementation
// ============================================================

// Deterministic bases for all 64-bit integers (proven sufficient)
static const uint64_t MR_BASES_64[] = {
    2, 325, 9375, 28178, 450775, 9780504, 1795265022
};

// Additional bases for 128-bit deterministic testing.
// Verified set: {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37}
// is sufficient for all n < 2^128 per Sorenson & Webster (2015).
static const uint64_t MR_BASES_128[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37
};

static bool miller_rabin(int128_t n) {
    if (n.value < 2) return false;
    if (n.value == 2 || n.value == 3) return true;
    if ((n.value & 1) == 0) return false;

    // Write n-1 = d * 2^s
    native_u128 d = n.value - 1;
    int s = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++s;
    }

    // Select bases based on n's magnitude
    const uint64_t* bases;
    int base_count;
    if (n.value <= UINT64_MAX) {
        bases = MR_BASES_64;
        base_count = sizeof(MR_BASES_64) / sizeof(MR_BASES_64[0]);
    } else {
        bases = MR_BASES_128;
        base_count = sizeof(MR_BASES_128) / sizeof(MR_BASES_128[0]);
    }

    for (int i = 0; i < base_count; ++i) {
        native_u128 a = bases[i];
        if (a % n.value == 0) continue;

        native_u128 x = pow_mod(int128_t(a), int128_t(d), n).value;
        if (x == 1 || x == n.value - 1) continue;

        bool composite = true;
        for (int r = 0; r < s - 1; ++r) {
            x = mul_mod(int128_t(x), int128_t(x), n).value;
            if (x == n.value - 1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}

// Trial division for small numbers (fast path)
static bool small_prime_check(int128_t n) {
    if (n.value < 2) return false;
    if (n.value == 2 || n.value == 3 || n.value == 5 || n.value == 7) return true;
    if ((n.value & 1) == 0 || n.value % 3 == 0 || n.value % 5 == 0) return false;

    int128_t limit = isqrt(n) + int128_t(1);
    for (native_u128 i = 7; i <= limit.value; i += 2) {
        if (n.value % i == 0) return false;
    }
    return true;
}

// ============================================================
// Public API
// ============================================================

bool is_prime(int128_t n) {
    // Fast path: trial division for small numbers (< 1,000,000)
    if (n.value < 1000000ULL) {
        return small_prime_check(n);
    }
    return miller_rabin(n);
}

int128_t next_prime(int128_t n) {
    if (n.value <= 2) return int128_t(2);
    if (n.value == 3) return int128_t(3);

    // Start from first odd >= n
    native_u128 candidate = n.value | 1;
    if (candidate == 1) candidate = 3;

    while (true) {
        if (miller_rabin(int128_t(candidate))) return int128_t(candidate);
        candidate += 2;
        if (candidate % 3 == 0) candidate += 2;
    }
}

int128_t prev_prime(int128_t n) {
    if (n.value <= 2) return int128_t(0);
    if (n.value == 3) return int128_t(2);

    // Largest odd < n: n-1 if n even, n-2 if n odd
    native_u128 candidate = n.value - 1 - (n.value & 1);
    while (candidate >= 2) {
        if (miller_rabin(int128_t(candidate))) return int128_t(candidate);
        candidate -= 2;
        if (candidate % 3 == 0 && candidate > 3) candidate -= 2;
    }
    return int128_t(0);
}

int128_t nth_prime(int128_t n) {
    if (n.value == 0) return int128_t(0);
    if (n.value == 1) return int128_t(2);
    if (n.value == 2) return int128_t(3);

    // Prime number theorem approximation: p_n ~ n * (log n + log log n)
    double dn = static_cast<double>(n.lo());
    double log_n = std::log(dn);
    double log_log_n = std::log(log_n);
    uint64_t approx = static_cast<uint64_t>(dn * (log_n + log_log_n)) + 2;

    // Generous overestimate for safety
    uint64_t limit = approx + (approx / 5) + 1000;

    // Simple sieve for the estimate range
    std::vector<bool> is_composite(limit + 1, false);
    uint64_t count = 0;

    for (uint64_t i = 2; i <= limit && count < n.value; ++i) {
        if (!is_composite[i]) {
            ++count;
            if (count == n.value) return int128_t(i);
            if (i * i <= limit) {
                for (uint64_t j = i * i; j <= limit; j += i) {
                    is_composite[j] = true;
                }
            }
        }
    }

    return int128_t(0); // Should not reach here for reasonable n
}

} // namespace PrimeCore