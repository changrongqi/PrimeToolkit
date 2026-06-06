// ============================================================
// sieve.cpp - Segmented sieve of Eratosthenes
// Works with int128_t range bounds; segment width limited to
// ~10^7 for memory efficiency.
// ============================================================

#include "sieve.h"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace PrimeCore {

// Generate small primes up to 'limit' using basic sieve.
// Used as base primes for the segmented sieve.
static std::vector<uint64_t> generate_small_primes(uint64_t limit) {
    std::vector<bool> is_prime(limit + 1, true);
    is_prime[0] = is_prime[1] = false;
    uint64_t sqrt_limit = static_cast<uint64_t>(std::sqrt(static_cast<double>(limit))) + 1;
    for (uint64_t i = 2; i <= sqrt_limit; ++i) {
        if (is_prime[i]) {
            for (uint64_t j = i * i; j <= limit; j += i) {
                is_prime[j] = false;
            }
        }
    }
    std::vector<uint64_t> primes;
    primes.reserve(static_cast<size_t>(limit / std::log(static_cast<double>(limit)) * 1.15));
    for (uint64_t i = 2; i <= limit; ++i) {
        if (is_prime[i]) primes.push_back(i);
    }
    return primes;
}

std::vector<int128_t> primes_in_range(int128_t from, int128_t to) {
    if (to < from) return {};
    if (to.value < 2) return {};
    if (from.value < 2) from = int128_t(2);

    // Range width must fit in memory
    native_u128 range_width = to.value - from.value;
    if (range_width > 10000000ULL) {
        // Return empty; caller should validate range before calling
        return {};
    }

    uint64_t u_from = from.lo();
    uint64_t u_to = to.lo();

    // For ranges fully within 64-bit, use the optimized path
    if (to.value <= UINT64_MAX) {
        std::vector<bool> is_prime(u_to + 1, true);
        is_prime[0] = is_prime[1] = false;
        uint64_t sqrt_to = static_cast<uint64_t>(std::sqrt(static_cast<double>(u_to))) + 1;
        for (uint64_t i = 2; i <= sqrt_to; ++i) {
            if (is_prime[i]) {
                for (uint64_t j = i * i; j <= u_to; j += i) {
                    is_prime[j] = false;
                }
            }
        }
        std::vector<int128_t> result;
        for (uint64_t i = u_from; i <= u_to; ++i) {
            if (is_prime[i]) result.emplace_back(i);
        }
        return result;
    }

    // Large range: segmented sieve
    // sqrt(to) for base primes
    uint64_t sqrt_to = 0;
    {
        // Approximate sqrt of 128-bit value
        // Use isqrt from int128_t
        int128_t approx_sqrt = int128_t(1);
        while (approx_sqrt * approx_sqrt < to) approx_sqrt = approx_sqrt + int128_t(1);
        sqrt_to = approx_sqrt.lo();
        if (sqrt_to > 100000000ULL) sqrt_to = 100000000ULL; // Cap for memory
    }

    auto base_primes = generate_small_primes(sqrt_to);

    constexpr uint64_t SEGMENT_SIZE = 262144ULL; // 256K
    uint64_t seg_len = static_cast<uint64_t>(range_width + 1);

    std::vector<int128_t> result;
    double estimate = static_cast<double>(seg_len) /
        std::log(static_cast<double>(to.value > 100 ? to.lo() : 100));
    result.reserve(static_cast<size_t>(estimate * 1.2) + 100);

    uint64_t seg_start = 0;
    while (seg_start < seg_len) {
        uint64_t current_len = std::min(SEGMENT_SIZE, seg_len - seg_start);
        std::vector<char> segment(static_cast<size_t>(current_len), 1);

        int128_t seg_abs_start = from + int128_t(seg_start);

        for (uint64_t p : base_primes) {
            native_u128 p2 = static_cast<native_u128>(p) * p;
            if (p2 > to.value) break;

            // Find first multiple of p in this segment
            native_u128 start = seg_abs_start.value / p * p;
            if (start < seg_abs_start.value) start += p;
            if (start < p2) start = p2;

            for (native_u128 j = start; j <= to.value && j < seg_abs_start.value + current_len; j += p) {
                size_t idx = static_cast<size_t>(j - seg_abs_start.value);
                if (idx < current_len) segment[idx] = 0;
            }
        }

        // Collect primes
        for (uint64_t i = 0; i < current_len; ++i) {
            if (segment[i]) {
                result.emplace_back(seg_abs_start.value + i);
            }
        }

        seg_start += current_len;
    }

    return result;
}

uint64_t primes_count(int128_t from, int128_t to) {
    if (to < from || to.value < 2) return 0;
    if (from.value < 2) from = int128_t(2);

    native_u128 range_width = to.value - from.value;

    // For 64-bit ranges, use optimized path
    if (to.value <= UINT64_MAX && range_width <= 10000000ULL) {
        uint64_t u_from = from.lo();
        uint64_t u_to = to.lo();
        std::vector<bool> is_prime(u_to + 1, true);
        is_prime[0] = is_prime[1] = false;
        uint64_t sqrt_to = static_cast<uint64_t>(std::sqrt(static_cast<double>(u_to))) + 1;
        for (uint64_t i = 2; i <= sqrt_to; ++i) {
            if (is_prime[i]) {
                for (uint64_t j = i * i; j <= u_to; j += i) {
                    is_prime[j] = false;
                }
            }
        }
        uint64_t count = 0;
        for (uint64_t i = u_from; i <= u_to; ++i) {
            if (is_prime[i]) ++count;
        }
        return count;
    }

    // Large range: segmented sieve count
    uint64_t sqrt_to = 0;
    {
        int128_t approx_sqrt = int128_t(1);
        while (approx_sqrt * approx_sqrt < to) approx_sqrt = approx_sqrt + int128_t(1);
        sqrt_to = approx_sqrt.lo();
        if (sqrt_to > 100000000ULL) sqrt_to = 100000000ULL;
    }

    auto base_primes = generate_small_primes(sqrt_to);

    constexpr uint64_t SEGMENT_SIZE = 262144ULL;
    uint64_t count = 0;
    uint64_t seg_len = static_cast<uint64_t>(std::min(range_width + 1, native_u128(UINT64_MAX)));

    uint64_t seg_start = 0;
    while (seg_start < seg_len) {
        uint64_t current_len = std::min(SEGMENT_SIZE, seg_len - seg_start);
        std::vector<char> segment(static_cast<size_t>(current_len), 1);

        int128_t seg_abs_start = from + int128_t(seg_start);

        for (uint64_t p : base_primes) {
            native_u128 p2 = static_cast<native_u128>(p) * p;
            if (p2 > to.value) break;

            native_u128 start = seg_abs_start.value / p * p;
            if (start < seg_abs_start.value) start += p;
            if (start < p2) start = p2;

            for (native_u128 j = start; j <= to.value && j < seg_abs_start.value + current_len; j += p) {
                size_t idx = static_cast<size_t>(j - seg_abs_start.value);
                if (idx < current_len) segment[idx] = 0;
            }
        }

        for (uint64_t i = 0; i < current_len; ++i) {
            if (segment[i]) ++count;
        }

        seg_start += current_len;
    }

    return count;
}

} // namespace PrimeCore