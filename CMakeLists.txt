cmake_minimum_required(VERSION 3.16)
project(PrimeToolkit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags for maximum speed
if(MSVC)
    add_compile_options(/O2 /arch:AVX2 /GL)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
else()
    add_compile_options(-O3 -march=native -flto)
endif()

# Source files grouped by module
set(CORE_SOURCES
    src/core/primality.cpp
    src/core/sieve.cpp
    src/core/factorization.cpp
)

set(SERVER_SOURCES
    src/server/http_server.cpp
)

set(APP_SOURCES
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