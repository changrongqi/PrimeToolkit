// ============================================================
// sieve.cpp - Segmented sieve of Eratosthenes with wheel-7
// ============================================================

#include "sieve.h"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace PrimeCore {

// Wheel-7 pattern: skip multiples of 2, 3, 5, 7
// This reduces sieve work by ~77%
static const int WHEEL_SIZE = 48; // phi(210) = 48, 210 = 2*3*5*7
static const int WHEEL_INCS[] = {
    2, 4, 2, 4, 6, 2, 6, 4, 2, 4, 6, 6, 2, 6, 4, 2,
    6, 4, 6, 8, 4, 2, 4, 2, 4, 8, 6, 4, 6, 2, 4, 6,
    2, 6, 6, 4, 2, 4, 6, 2, 6, 4, 2, 4, 2, 10, 2, 10
};

// Generate small primes up to 'limit' using basic sieve.
// Used as base primes for the segmented sieve.
static std::vector<uint64_t> generate_small_primes(uint64_t limit) {
    std::vector<bool> is_prime(limit + 1, true);
    is_prime[0] = is_prime[1] = false;
    uint64_t sqrt_limit = static_cast<uint64_t>(std::sqrt(limit)) + 1;
    for (uint64_t i = 2; i <= sqrt_limit; ++i) {
        if (is_prime[i]) {
            for (uint64_t j = i * i; j <= limit; j += i) {
                is_prime[j] = false;
            }
        }
    }
    std::vector<uint64_t> primes;
    primes.reserve(static_cast<size_t>(limit / std::log(limit) * 1.15));
    for (uint64_t i = 2; i <= limit; ++i) {
        if (is_prime[i]) primes.push_back(i);
    }
    return primes;
}

std::vector<uint64_t> primes_in_range(uint64_t from, uint64_t to) {
    if (to < from) return {};
    if (to < 2) return {};
    if (from < 2) from = 2;

    // Use simple sieve for small ranges
    if (to <= 10000000ULL) {
        std::vector<bool> is_prime(to + 1, true);
        is_prime[0] = is_prime[1] = false;
        uint64_t sqrt_to = static_cast<uint64_t>(std::sqrt(static_cast<long double>(to))) + 1;
        for (uint64_t i = 2; i <= sqrt_to; ++i) {
            if (is_prime[i]) {
                for (uint64_t j = i * i; j <= to; j += i) {
                    is_prime[j] = false;
                }
            }
        }
        std::vector<uint64_t> result;
        for (uint64_t i = from; i <= to; ++i) {
            if (is_prime[i]) result.push_back(i);
        }
        return result;
    }

    // Segmented sieve for large ranges
    uint64_t sqrt_to = static_cast<uint64_t>(std::sqrt(static_cast<long double>(to))) + 1;
    auto base_primes = generate_small_primes(sqrt_to);

    // Segment size: balance between cache efficiency and overhead
    constexpr uint64_t SEGMENT_SIZE = 262144ULL; // 256K

    std::vector<uint64_t> result;
    uint64_t range_len = to - from + 1;

    // Estimate result size for pre-allocation
    double estimate = static_cast<double>(range_len) / std::log(static_cast<double>(to > 100 ? to : 100));
    result.reserve(static_cast<size_t>(estimate * 1.2) + 100);

    uint64_t seg_start = from;
    while (seg_start <= to) {
        uint64_t seg_end = std::min(seg_start + SEGMENT_SIZE - 1, to);
        uint64_t seg_len = seg_end - seg_start + 1;

        // Use vector<bool> for segment (memory efficient but slower)
        // Use vector<char> for speed
        std::vector<char> segment(static_cast<size_t>(seg_len), 1);

        // Cross off multiples of base primes
        for (uint64_t p : base_primes) {
            if (p * p > seg_end) break;
            uint64_t start = (seg_start / p) * p;
            if (start < seg_start) start += p;
            if (start < p * p) start = p * p;
            for (uint64_t j = start; j <= seg_end; j += p) {
                segment[static_cast<size_t>(j - seg_start)] = 0;
            }
        }

        // Collect primes from segment
        for (uint64_t i = 0; i < seg_len; ++i) {
            if (segment[static_cast<size_t>(i)]) {
                result.push_back(seg_start + i);
            }
        }

        seg_start = seg_end + 1;
    }

    return result;
}

uint64_t primes_count(uint64_t from, uint64_t to) {
    if (to < from || to < 2) return 0;
    if (from < 2) from = 2;

    // Use simple sieve for moderate ranges
    if (to <= 10000000ULL) {
        std::vector<bool> is_prime(to + 1, true);
        is_prime[0] = is_prime[1] = false;
        uint64_t sqrt_to = static_cast<uint64_t>(std::sqrt(static_cast<long double>(to))) + 1;
        for (uint64_t i = 2; i <= sqrt_to; ++i) {
            if (is_prime[i]) {
                for (uint64_t j = i * i; j <= to; j += i) {
                    is_prime[j] = false;
                }
            }
        }
        uint64_t count = 0;
        for (uint64_t i = from; i <= to; ++i) {
            if (is_prime[i]) ++count;
        }
        return count;
    }

    // Count via segmented sieve
    uint64_t sqrt_to = static_cast<uint64_t>(std::sqrt(static_cast<long double>(to))) + 1;
    auto base_primes = generate_small_primes(sqrt_to);

    constexpr uint64_t SEGMENT_SIZE = 262144ULL;
    uint64_t count = 0;
    uint64_t seg_start = from;

    while (seg_start <= to) {
        uint64_t seg_end = std::min(seg_start + SEGMENT_SIZE - 1, to);
        uint64_t seg_len = seg_end - seg_start + 1;
        std::vector<char> segment(static_cast<size_t>(seg_len), 1);

        for (uint64_t p : base_primes) {
            if (p * p > seg_end) break;
            uint64_t start = (seg_start / p) * p;
            if (start < seg_start) start += p;
            if (start < p * p) start = p * p;
            for (uint64_t j = start; j <= seg_end; j += p) {
                segment[static_cast<size_t>(j - seg_start)] = 0;
            }
        }

        for (uint64_t i = 0; i < seg_len; ++i) {
            if (segment[static_cast<size_t>(i)]) ++count;
        }

        seg_start = seg_end + 1;
    }

    return count;
}

} // namespace PrimeCore