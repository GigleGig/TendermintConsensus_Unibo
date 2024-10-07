set(CMAKE_C_COMPILER "F:/msys2/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "F:/msys2/mingw64/bin/g++.exe")

cmake_minimum_required(VERSION 3.10)
project(TendermintConsensus)

set(CMAKE_CXX_STANDARD 17)

# Add openssl path
set(OPENSSL_ROOT_DIR "F:/Visual Studio/CppCode/Crypto/openssl/lib/x64/Release")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
set(OPENSSL_LIB_DIR "${OPENSSL_ROOT_DIR}/lib")

# Add source code
file(GLOB SOURCES "src/*.cpp")

add_executable(TendermintConsensus main.cpp ${SOURCES})

include_directories(src ${OPENSSL_INCLUDE_DIR})

# Link openssl library
target_link_libraries(TendermintConsensus PRIVATE "${OPENSSL_LIB_DIR}/libssl.lib" "${OPENSSL_LIB_DIR}/libcrypto.lib" Crypt32.lib Ws2_32.lib)

# Add Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

# Add test dest
file(GLOB TEST_SOURCES "tests/*.cpp")
add_executable(runTests ${TEST_SOURCES} ${SOURCES})

target_link_libraries(runTests PRIVATE gtest_main "${OPENSSL_LIB_DIR}/libssl.lib" "${OPENSSL_LIB_DIR}/libcrypto.lib" Crypt32.lib Ws2_32.lib)

include(GoogleTest)
gtest_discover_tests(runTests)
