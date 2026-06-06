cmake_minimum_required(VERSION 3.16)
project(PrimeToolkit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags for maximum speed (x86-64 for cross-CPU compatibility)
if(MSVC)
    add_compile_options(/O2 /GL)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
else()
    add_compile_options(-O3 -march=x86-64)
endif()

# Source files grouped by module
set(CORE_SOURCES
    src/core/int128_t.cpp
    src/core/primality.cpp
    src/core/sieve.cpp
    src/core/factorization.cpp
)

set(SERVER_SOURCES
    src/server/http_server.cpp
)

set(APP_SOURCES
    src/api.cpp
    src/main.cpp
)

# Build executable
add_executable(PrimeToolkit
    ${CORE_SOURCES}
    ${SERVER_SOURCES}
    ${APP_SOURCES}
)

# Include directories
target_include_directories(PrimeToolkit PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Warning flags
if(MSVC)
    target_compile_options(PrimeToolkit PRIVATE /W4)
else()
    target_compile_options(PrimeToolkit PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Link Windows libraries
if(WIN32)
    target_link_libraries(PrimeToolkit PRIVATE ws2_32 shell32)
endif()

# Copy web directory to output
add_custom_command(TARGET PrimeToolkit POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/web
    $<TARGET_FILE_DIR:PrimeToolkit>/web
    COMMENT "Copying web/ to build output...")

# Output in build/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)