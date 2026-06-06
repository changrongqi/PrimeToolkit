// sieve.cpp  --  Segmented sieve of Eratosthenes
// Copyright (c) 2024 PrimeToolkit Project
//
// Generates and counts primes in 128-bit ranges.
// Uses segmented sieve with 1 MB segment size for optimal
// L2 cache usage. Also provides nth-prime lookup via prime
// number theorem approximation.

#include <algorithm>
#include <cmath>
#include <vector>

#include "core/sieve.h"

namespace PrimeCore {

// ============================================================
// Base prime generation (small primes up to sqrt of range)
// ============================================================
static std::vector<uint64_t> generate_small_primes(uint64_t limit) {
    std::vector<bool> is_prime(limit + 1, true);
    is_prime[0] = is_prime[1] = false;
    uint64_t sqrt_lim =
        static_cast<uint64_t>(std::sqrt(static_cast<double>(limit))) + 1;
    for (uint64_t i = 2; i <= sqrt_lim; ++i) {
        if (is_prime[i]) {
            for (uint64_t j = i * i; j <= limit; j += i) {
                is_prime[j] = false;
            }
        }
    }
    std::vector<uint64_t> primes;
    double estimate = static_cast<double>(limit) /
        std::log(static_cast<double>(limit)) * 1.15;
    primes.reserve(static_cast<size_t>(estimate));
    for (uint64_t i = 2; i <= limit; ++i) {
        if (is_prime[i]) primes.push_back(i);
    }
    return primes;
}

// ============================================================
// Segment processing (shared by primes_in_range and primes_count)
// ============================================================

// 1 MB segment = 1,048,576 bytes, optimal for L2 cache
static constexpr uint64_t SEGMENT_SIZE = 1048576ULL;

template <typename Collector>
static void process_segment(const std::vector<uint64_t>& base_primes,
                            int128_t from, int128_t to,
                            uint64_t seg_len, Collector& collector) {
    uint64_t seg_start = 0;
    while (seg_start < seg_len) {
        uint64_t cur_len = std::min(SEGMENT_SIZE, seg_len - seg_start);
        std::vector<char> segment(static_cast<size_t>(cur_len), 1);

        int128_t seg_abs_start = from + int128_t(seg_start);

        for (uint64_t p : base_primes) {
            native_u128 p2 = static_cast<native_u128>(p) * p;
            if (p2 > to.value) break;

            native_u128 start = seg_abs_start.value / p * p;
            if (start < seg_abs_start.value) start += p;
            if (start < p2) start = p2;

            native_u128 seg_end = seg_abs_start.value + cur_len;
            for (native_u128 j = start;
                 j <= to.value && j < seg_end; j += p) {
                size_t idx = static_cast<size_t>(
                    j - seg_abs_start.value);
                segment[idx] = 0;
            }
        }

        for (uint64_t i = 0; i < cur_len; ++i) {
            if (segment[i]) collector.collect(seg_abs_start.value + i);
        }

        seg_start += cur_len;
    }
}

// ============================================================
// primes_in_range: generate all primes in [from, to]
// ============================================================
std::vector<int128_t> primes_in_range(int128_t from, int128_t to) {
    if (to < from) return {};
    if (to.value < 2) return {};
    if (from.value < 2) from = int128_t(2);

    native_u128 range_width = to.value - from.value;
    if (range_width > 10000000ULL) return {};

    uint64_t u_from = from.lo();
    uint64_t u_to = to.lo();

    // For 64-bit ranges, sieve only the needed range
    if (to.value <= UINT64_MAX) {
        uint64_t seg_len = static_cast<uint64_t>(range_width + 1);
        std::vector<char> is_prime(static_cast<size_t>(seg_len), 1);

        uint64_t sqrt_to =
            static_cast<uint64_t>(std::sqrt(static_cast<double>(u_to))) + 1;
        std::vector<bool> base_prime(sqrt_to + 1, true);
        base_prime[0] = base_prime[1] = false;
        for (uint64_t i = 2; i * i <= sqrt_to; ++i) {
            if (base_prime[i]) {
                for (uint64_t j = i * i; j <= sqrt_to; j += i) {
                    base_prime[j] = false;
                }
            }
        }

        for (uint64_t p = 2; p <= sqrt_to; ++p) {
            if (!base_prime[p]) continue;
            uint64_t start = (u_from / p) * p;
            if (start < u_from) start += p;
            if (start < p * p) start = p * p;
            if (start > u_to) continue;
            for (uint64_t j = start; j <= u_to; j += p) {
                is_prime[j - u_from] = 0;
            }
        }

        std::vector<int128_t> result;
        for (uint64_t i = 0; i < seg_len; ++i) {
            if (is_prime[i] && (u_from + i) >= 2) {
                result.emplace_back(u_from + i);
            }
        }
        return result;
    }

    // Large range: segmented sieve
    uint64_t sqrt_to = isqrt128(to).lo();
    if (sqrt_to > 100000000ULL) sqrt_to = 100000000ULL;

    auto base_primes = generate_small_primes(sqrt_to);
    uint64_t seg_len = static_cast<uint64_t>(range_width + 1);

    struct ListCollector {
        std::vector<int128_t>& result;
        void collect(native_u128 v) { result.emplace_back(v); }
    };
    std::vector<int128_t> result;
    double estimate = static_cast<double>(seg_len) /
        std::log(static_cast<double>(to.value > 100 ? to.lo() : 100));
    result.reserve(static_cast<size_t>(estimate * 1.2) + 100);

    ListCollector collector{result};
    process_segment(base_primes, from, to, seg_len, collector);
    return result;
}

// ============================================================
// primes_count: count primes in [from, to]
// ============================================================
uint64_t primes_count(int128_t from, int128_t to) {
    if (to < from || to.value < 2) return 0;
    if (from.value < 2) from = int128_t(2);

    native_u128 range_width = to.value - from.value;

    // For 64-bit ranges, use optimized path
    if (to.value <= UINT64_MAX && range_width <= 10000000ULL) {
        uint64_t u_from = from.lo();
        uint64_t u_to = to.lo();
        uint64_t seg_len = static_cast<uint64_t>(range_width + 1);
        std::vector<char> is_prime(static_cast<size_t>(seg_len), 1);

        uint64_t sqrt_to =
            static_cast<uint64_t>(std::sqrt(static_cast<double>(u_to))) + 1;
        std::vector<bool> base_prime(sqrt_to + 1, true);
        base_prime[0] = base_prime[1] = false;
        for (uint64_t i = 2; i * i <= sqrt_to; ++i) {
            if (base_prime[i]) {
                for (uint64_t j = i * i; j <= sqrt_to; j += i) {
                    base_prime[j] = false;
                }
            }
        }

        for (uint64_t p = 2; p <= sqrt_to; ++p) {
            if (!base_prime[p]) continue;
            uint64_t start = (u_from / p) * p;
            if (start < u_from) start += p;
            if (start < p * p) start = p * p;
            if (start > u_to) continue;
            for (uint64_t j = start; j <= u_to; j += p) {
                is_prime[j - u_from] = 0;
            }
        }

        uint64_t count = 0;
        for (uint64_t i = 0; i < seg_len; ++i) {
            if (is_prime[i] && (u_from + i) >= 2) ++count;
        }
        return count;
    }

    // Large range: segmented sieve count
    uint64_t sqrt_to = isqrt128(to).lo();
    if (sqrt_to > 100000000ULL) sqrt_to = 100000000ULL;

    auto base_primes = generate_small_primes(sqrt_to);
    uint64_t seg_len = static_cast<uint64_t>(
        std::min(range_width + 1, native_u128(UINT64_MAX)));

    struct CountCollector {
        uint64_t count = 0;
        void collect(native_u128) { ++count; }
    };
    CountCollector collector;
    process_segment(base_primes, from, to, seg_len, collector);
    return collector.count;
}

// ============================================================
// nth_prime: find the n-th prime number
// ============================================================
int128_t nth_prime(int128_t n) {
    if (n.value == 0) return int128_t(0);
    if (n.value == 1) return int128_t(2);
    if (n.value == 2) return int128_t(3);

    // Prime number theorem: p_n ~ n * (log n + log log n)
    double dn = static_cast<double>(n.lo());
    double log_n = std::log(dn);
    double log_log_n = std::log(log_n);
    uint64_t approx =
        static_cast<uint64_t>(dn * (log_n + log_log_n)) + 2;

    // Generous overestimate for safety
    uint64_t limit = approx + (approx / 5) + 1000;

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

    return int128_t(0);
}

}  // namespace PrimeCore
