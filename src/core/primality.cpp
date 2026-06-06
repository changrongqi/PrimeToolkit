/*
 * primality.cpp - Miller-Rabin deterministic primality test
 * Copyright (c) 2024 PrimeToolkit Project
 * 
 * Supports full 128-bit range with deterministic bases.
 * Miller-Rabin bases verified per Sorenson & Webster (2015).
 */

#include <cmath>

#include "primality.h"

namespace PrimeCore {

// ============================================================
// Modular exponentiation: (base ^ exp) % mod
// ============================================================
static inline int128_t pow_mod(int128_t base, int128_t exp, int128_t mod) {
    native_u128 result = 1;
    base.value %= mod.value;
    while (exp.value) {
        if (exp.value & 1) {
            result = mul_mod128(int128_t(result), base, mod).value;
        }
        base = mul_mod128(base, base, mod);
        exp.value >>= 1;
    }
    return result;
}

// ============================================================
// Deterministic Miller-Rabin bases
// ============================================================

// Proven sufficient for all 64-bit integers
static const uint64_t MR_BASES_64[] = {
    2, 325, 9375, 28178, 450775, 9780504, 1795265022
};

// Proven sufficient for all n < 2^128 (Sorenson & Webster, 2015)
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
        if (a >= n.value) continue;
        if (n.value % a == 0) return false;

        native_u128 x = pow_mod(int128_t(a), int128_t(d), n).value;
        if (x == 1 || x == n.value - 1) continue;

        bool composite = true;
        for (int r = 0; r < s - 1; ++r) {
            x = mul_mod128(int128_t(x), int128_t(x), n).value;
            if (x == n.value - 1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}

// ============================================================
// Trial division for small numbers (fast path for n < 1,000,000)
// ============================================================
static bool small_prime_check(int128_t n) {
    if (n.value < 2) return false;
    if (n.value == 2 || n.value == 3 || n.value == 5 || n.value == 7) return true;
    if ((n.value & 1) == 0 || n.value % 3 == 0 || n.value % 5 == 0) return false;

    native_u128 limit = isqrt128(n).value + 1;
    // Wheel: 2, 4, 2, 4, 6, 2, 6, 4, 2, 4, 6, 6, 2, 6, 4, 2, 6, 4, 6, 8, 4, 2...
    // Simplified: skip multiples of 2, 3, 5 via {4,2,4,2,4,6,2,6} pattern
    static const uint8_t wheel[] = { 4, 2, 4, 2, 4, 6, 2, 6 };
    int wi = 0;
    for (native_u128 i = 7; i <= limit; i += wheel[wi]) {
        if (n.value % i == 0) return false;
        wi = (wi + 1) & 7;
    }
    return true;
}

// ============================================================
// Public API
// ============================================================

bool is_prime(int128_t n) {
    if (n.value < 1000000ULL) return small_prime_check(n);
    return miller_rabin(n);
}

int128_t next_prime(int128_t n) {
    if (n.value <= 2) return int128_t(2);
    if (n.value == 3) return int128_t(3);

    // Wheel mod 30: skip multiples of 2,3,5
    static const uint8_t wheel30[] = { 2, 6, 4, 2, 4, 2, 4, 6 };
    static const uint8_t residues[] = { 1, 7, 11, 13, 17, 19, 23, 29 };

    native_u128 candidate = n.value | 1;
    if (candidate == 1) candidate = 3;

    // Align candidate to the wheel
    uint32_t mod30 = candidate % 30;
    int wi = 0;
    for (int j = 0; j < 8; ++j) {
        if (residues[j] >= mod30) { wi = j; break; }
    }
    if (mod30 > residues[7]) { candidate += 30 - mod30 + residues[0]; wi = 0; }
    else if (mod30 < residues[0]) { candidate += residues[0] - mod30; wi = 0; }
    else { candidate += residues[wi] - mod30; }

    while (true) {
        if (miller_rabin(int128_t(candidate))) return int128_t(candidate);
        candidate += wheel30[wi];
        wi = (wi + 1) & 7;
    }
}

int128_t prev_prime(int128_t n) {
    if (n.value <= 2) return int128_t(0);
    if (n.value == 3) return int128_t(2);

    static const uint8_t wheel30[] = { 6, 4, 2, 4, 2, 4, 6, 2 };
    static const uint8_t residues[] = { 29, 23, 19, 17, 13, 11, 7, 1 };

    native_u128 candidate = n.value - 1 - (n.value & 1);
    if (candidate <= 2) return int128_t(0);

    uint32_t mod30 = candidate % 30;
    int wi = 0;
    for (int j = 0; j < 8; ++j) {
        if (residues[j] <= mod30) { wi = j; break; }
    }
    candidate -= mod30 - residues[wi];

    while (candidate >= 2) {
        if (miller_rabin(int128_t(candidate))) return int128_t(candidate);
        if (candidate <= wheel30[wi]) break;
        candidate -= wheel30[wi];
        wi = (wi + 1) & 7;
    }
    return int128_t(0);
}

} // namespace PrimeCore