#!/bin/bash
# build.sh - Build script for PrimeToolkit (Linux/macOS)
# Usage: ./build.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}[*] Building PrimeToolkit${NC}"
echo

# Create output directory
mkdir -p build/bin

# Check for g++
if ! command -v g++ &> /dev/null; then
    echo -e "${RED}[!] g++ not found. Install with:${NC}"
    echo "  Ubuntu/Debian:  sudo apt install build-essential"
    echo "  macOS:          brew install gcc"
    exit 1
fi

# Detect compiler
COMPILER="${CXX:-g++}"
echo -e "[*] Using: ${COMPILER} ($( ${COMPILER} --version | head -n1 ))"

# Compile
echo -e "[1/2] Compiling..."
"${COMPILER}" -std=c++17 -O3 -march=x86-64 \
    -o "build/bin/PrimeToolkit" \
    src/core/int128_t.cpp \
    src/core/primality.cpp \
    src/core/sieve.cpp \
    src/core/factorization.cpp \
    src/api.cpp \
    src/server/http_server.cpp \
    src/main.cpp \
    -I src \
    -pthread \
    -lcurl \
    2>&1

if [ $? -eq 0 ]; then
    echo -e "[2/2] ${GREEN}Build complete!${NC}"
    echo
    echo -e "[+] ${GREEN}build/bin/PrimeToolkit created${NC}"
    echo
    echo "Run: ./build/bin/PrimeToolkit"
    echo
else
    echo -e "${RED}[!] Build failed${NC}"
    exit 1
fi

# Copy web/ to output
if [ -d "web" ]; then
    cp -r web build/bin/
    echo -e "[+] web/ copied to build/bin/"
fi
