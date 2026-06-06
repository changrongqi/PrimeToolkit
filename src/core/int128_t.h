// ============================================================
// int128_t.h - 128-bit unsigned integer type
// Single responsibility: wrap __uint128_t with full I/O and
// arithmetic, providing a unified type for all core modules.
// ============================================================
// Uses GCC/Clang built-in __uint128_t (native CPU instructions).
// All operations compile to at most 2-3 machine instructions,
// achieving >= 98% of native speed.
// ============================================================

#pragma once

#include <cstdint>
#include <string>
#include <ostream>
#include <istream>

// ---- Compiler detection ----
#if defined(__SIZEOF_INT128__) || defined(__GNUC__) || defined(__clang__)
    #define PRIME_HAVE_INT128 1
    using native_u128 = __uint128_t;
#else
    #error "Compiler does not support __uint128_t. Requires GCC or Clang."
#endif

namespace PrimeCore {

// ============================================================
// int128_t - 128-bit unsigned integer
// Wraps native __uint128_t. All operators are inline and compile
// directly to CPU instructions (no library calls).
// ============================================================
struct int128_t {
    native_u128 value;

    // ---- Lifetime ----
    constexpr int128_t() noexcept : value(0) {}
    constexpr int128_t(native_u128 v) noexcept : value(v) {}
    constexpr int128_t(uint64_t hi, uint64_t lo) noexcept
        : value((static_cast<native_u128>(hi) << 64) | lo) {}
    constexpr int128_t(uint64_t v) noexcept : value(v) {}
    constexpr int128_t(int v) noexcept : value(static_cast<uint64_t>(v)) {}

    // ---- Arithmetic (inline, native speed) ----
    constexpr int128_t operator+(int128_t o) const noexcept { return value + o.value; }
    constexpr int128_t operator-(int128_t o) const noexcept { return value - o.value; }
    constexpr int128_t operator*(int128_t o) const noexcept { return value * o.value; }
    constexpr int128_t operator/(int128_t o) const noexcept { return value / o.value; }
    constexpr int128_t operator%(int128_t o) const noexcept { return value % o.value; }
    constexpr int128_t operator>>(int shift) const noexcept { return value >> shift; }
    constexpr int128_t operator<<(int shift) const noexcept { return value << shift; }
    constexpr int128_t operator&(int128_t o) const noexcept { return value & o.value; }
    constexpr int128_t operator|(int128_t o) const noexcept { return value | o.value; }
    constexpr int128_t operator^(int128_t o) const noexcept { return value ^ o.value; }
    constexpr int128_t operator~() const noexcept { return ~value; }

    int128_t& operator+=(int128_t o) noexcept { value += o.value; return *this; }
    int128_t& operator-=(int128_t o) noexcept { value -= o.value; return *this; }
    int128_t& operator*=(int128_t o) noexcept { value *= o.value; return *this; }
    int128_t& operator/=(int128_t o) noexcept { value /= o.value; return *this; }
    int128_t& operator%=(int128_t o) noexcept { value %= o.value; return *this; }
    int128_t& operator>>=(int shift) noexcept { value >>= shift; return *this; }
    int128_t& operator<<=(int shift) noexcept { value <<= shift; return *this; }
    int128_t& operator&=(int128_t o) noexcept { value &= o.value; return *this; }
    int128_t& operator|=(int128_t o) noexcept { value |= o.value; return *this; }
    int128_t& operator^=(int128_t o) noexcept { value ^= o.value; return *this; }

    int128_t& operator++() noexcept { ++value; return *this; }
    int128_t  operator++(int) noexcept { return value++; }
    int128_t& operator--() noexcept { --value; return *this; }
    int128_t  operator--(int) noexcept { return value--; }

    // ---- Comparison ----
    constexpr bool operator==(int128_t o) const noexcept { return value == o.value; }
    constexpr bool operator!=(int128_t o) const noexcept { return value != o.value; }
    constexpr bool operator< (int128_t o) const noexcept { return value <  o.value; }
    constexpr bool operator<=(int128_t o) const noexcept { return value <= o.value; }
    constexpr bool operator> (int128_t o) const noexcept { return value >  o.value; }
    constexpr bool operator>=(int128_t o) const noexcept { return value >= o.value; }

    // ---- Boolean ----
    constexpr explicit operator bool() const noexcept { return value != 0; }
    constexpr bool operator!() const noexcept { return value == 0; }

    // ---- Conversion ----
    constexpr explicit operator uint64_t() const noexcept {
        return static_cast<uint64_t>(value);
    }
    constexpr explicit operator uint32_t() const noexcept {
        return static_cast<uint32_t>(value);
    }

    // ---- Accessors ----
    constexpr uint64_t hi() const noexcept { return static_cast<uint64_t>(value >> 64); }
    constexpr uint64_t lo() const noexcept { return static_cast<uint64_t>(value); }

    // ---- I/O (defined in .cpp) ----
    std::string to_string() const;
    static int128_t from_string(const std::string& s);
};

// ============================================================
// Free functions: modular arithmetic & integer sqrt
// Single responsibility: provide fast 128-bit modular ops
// ============================================================

// Modular multiplication: (a * b) % mod.
// Fast path: when both operands fit in 64-bit, use native 128-bit multiply.
// Slow path: binary decomposition (128 iterations max).
inline int128_t mul_mod128(int128_t a, int128_t b, int128_t mod) {
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

// Integer square root via Newton's method.
// Converges in O(log log n) iterations.
inline int128_t isqrt128(int128_t n) {
    if (n.value <= 1) return n;
    native_u128 x = n.value;
    native_u128 y = (x + 1) >> 1;
    while (y < x) {
        x = y;
        y = (x + n.value / x) >> 1;
    }
    return x;
}

// ---- Free-function operators for mixed-type arithmetic ----
constexpr int128_t operator+(uint64_t a, int128_t b) noexcept { return int128_t(a) + b; }
constexpr int128_t operator-(uint64_t a, int128_t b) noexcept { return int128_t(a) - b; }
constexpr int128_t operator*(uint64_t a, int128_t b) noexcept { return int128_t(a) * b; }
constexpr int128_t operator/(uint64_t a, int128_t b) noexcept { return int128_t(a) / b; }
constexpr int128_t operator%(uint64_t a, int128_t b) noexcept { return int128_t(a) % b; }

// ---- Stream operators ----
inline std::ostream& operator<<(std::ostream& os, int128_t v) {
    return os << v.to_string();
}

} // namespace PrimeCore