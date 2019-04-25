cmake_minimum_required(VERSION 2.8)
CMAKE_policy(SET CMP0015 NEW)

project(vodsdk)
# SET(BOOST_HEADER_DIR "/root/boost_1_61_0")
SET(CMAKE_VERBOSE_MAKEFILE  ON)
# find_package自动查找，或者通过SET(OPENSSL_VERSION 1.0.0)人工设置

find_package(OpenSSL REQUIRED)
#SET(OPENSSL_VERSION 1.0.1e)
message(${OPENSSL_VERSION})

# 单元测试与覆盖率统计
OPTION (ENABLE_UNITTEST "Unittest" OFF)

# include directories
INCLUDE_DIRECTORIES(D:\\cossdk\\dependence\\include)
INCLUDE_DIRECTORIES(D:\\cossdk\\cos-cpp-sdk-v5-fixbug\\include)
INCLUDE_DIRECTORIES(D:\\cossdk\\vodsdk\\vodsdk\\vodsdk)
# include_directories(${BOOST_HEADER_DIR})

# lib directories
link_directories(D:\\cossdk\\vodsdk\\vodsdk\\2017-1\\lib\\Release)
link_directories(D:\\cossdk\\cos-cpp-sdk-v5-fixbug\\2017\\lib\\Release)
link_directories(D:\\cossdk\\dependence\\lib)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


ADD_SUBDIRECTORY(vodsdk)
ADD_SUBDIRECTORY(demo)