cmake_minimum_required(VERSION 3.18)
project(QuasarPLO LANGUAGES CXX)

option(QUASAR_BUILD_TESTS "Build C++ tests" ON)
option(QUASAR_BUILD_PYBIND "Build pybind11 module" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(engine)

if (QUASAR_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

