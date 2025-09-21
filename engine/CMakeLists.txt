add_library(quasar_engine
  src/version.cpp
  src/public_state.cpp
)

target_include_directories(quasar_engine
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(quasar_engine PUBLIC cxx_std_17)

if (QUASAR_BUILD_PYBIND)
  find_package(pybind11 QUIET)
  if (pybind11_FOUND)
    add_subdirectory(pybind)
  else()
    message(WARNING "pybind11 not found; skipping Python module")
  endif()
endif()

