# Prime Number Toolkit

A high-performance prime number toolkit with C++ core and elegant web UI.

## Features

- **Primality Test**: Deterministic Miller-Rabin for all 64-bit integers (instant result)
- **Prime Generation**: Segmented sieve for generating primes in ranges (max 10M)
- **Prime Counting**: Optimized counting algorithm for large ranges
- **Prime Factorization**: Trial division + Pollard's Rho (Brent variant)
- **Nth Prime**: Find the n-th prime (supports up to n = 10,000,000)
- **Multi-language**: English / 中文 support with persistent language preference

## Tech Stack

- **Backend**: C++17, Winsock2 (built-in HTTP server)
- **Frontend**: HTML5, CSS3, Vanilla JavaScript
- **Build**: MinGW / MSVC compatible (auto-detection)

## Requirements

- **OS**: Windows 10/11 (64-bit)
- **Compiler**: MinGW-w64 (g++) or Visual Studio 2022 with C++ tools
  - MinGW: https://www.mingw-w64.org/
  - VS2022: https://visualstudio.microsoft.com/downloads/

## Performance Highlights

- Miller-Rabin with 7 deterministic bases (proven correct for all 64-bit integers)
- Segmented sieve with 256KB block size for optimal cache usage
- Pollard's Rho with Brent cycle detection for fast factorization
- Binary GCD for efficient greatest common divisor computation

## Quick Start

```bash
# Build (Windows)
.\build.bat

# Run (auto-starts server and opens browser)
.\build\bin\PrimeToolkit.exe
```

> Note: The executable is located in `build\bin\` after building.

Visit `http://localhost:8080` in your browser.

## Project Structure

```
src/
├── core/          # Prime algorithms
│   ├── primality.cpp      # Miller-Rabin primality test
│   ├── sieve.cpp          # Segmented sieve
│   └── factorization.cpp  # Pollard's Rho factorization
├── server/        # HTTP server (Winsock2)
└── web/           # Frontend UI (i18n support)
```

## API Endpoints

| Endpoint | Description |
|----------|-------------|
| `/api/is_prime?n=97` | Check if n is prime |
| `/api/next_prime?n=100` | Find next prime after n |
| `/api/prev_prime?n=100` | Find previous prime before n |
| `/api/primes?from=1&to=50` | Generate primes in range |
| `/api/primes_count?from=1&to=1000` | Count primes in range |
| `/api/factorize?n=123456` | Factorize n into primes |
| `/api/nth_prime?n=100` | Find the 100th prime |

## License

MIT License