cmake_minimum_required(VERSION 3.15)
project(tripplex_dll VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(tripplex SHARED src/tripplex.cpp)

target_include_directories(tripplex PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

if (WIN32)
  target_compile_definitions(tripplex PRIVATE TPX_EXPORTS)
  target_link_libraries(tripplex PRIVATE ws2_32)
endif()

find_package(OpenSSL QUIET)
if(OpenSSL_FOUND)
  target_compile_definitions(tripplex PRIVATE USE_OPENSSL)
  target_include_directories(tripplex PRIVATE ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(tripplex PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()
