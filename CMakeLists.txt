set(CMAKE_C_COMPILER "F:/msys2/mingw64/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "F:/msys2/mingw64/bin/g++.exe")

cmake_minimum_required(VERSION 3.10)
project(TendermintConsensus)

set(CMAKE_CXX_STANDARD 17)

# 添加 OpenSSL 库的路径
set(OPENSSL_ROOT_DIR "F:/Visual Studio/CppCode/Crypto/openssl/lib/x64/Release")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
set(OPENSSL_LIB_DIR "${OPENSSL_ROOT_DIR}/lib")

# 添加源代码
file(GLOB SOURCES "src/*.cpp")

# 添加可执行目标
add_executable(TendermintConsensus main.cpp ${SOURCES})

# 包含头文件目录
include_directories(src ${OPENSSL_INCLUDE_DIR})

# 链接 OpenSSL 库
target_link_libraries(TendermintConsensus PRIVATE "${OPENSSL_LIB_DIR}/libssl.lib" "${OPENSSL_LIB_DIR}/libcrypto.lib" Crypt32.lib Ws2_32.lib)

# 添加 Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

# 添加测试目标，确保所有源文件都被包含
file(GLOB TEST_SOURCES "tests/*.cpp")
add_executable(runTests ${TEST_SOURCES} ${SOURCES})

# 链接 Google Test 和 OpenSSL 库
target_link_libraries(runTests PRIVATE gtest_main "${OPENSSL_LIB_DIR}/libssl.lib" "${OPENSSL_LIB_DIR}/libcrypto.lib" Crypt32.lib Ws2_32.lib)

include(GoogleTest)
gtest_discover_tests(runTests)
