add_library(quasar_engine
  src/version.cpp
  src/public_state.cpp
  src/plo_legal.cpp
  src/discretize.cpp
  src/json/parse_spot.cpp
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

# CLI tool to print legal actions for a JSON-described spot
add_executable(quasar_cli src/cli/main.cpp)
target_link_libraries(quasar_cli PRIVATE quasar_engine)
target_compile_features(quasar_cli PRIVATE cxx_std_17)

# Optional TorchScript wrapper
option(QUASAR_BUILD_TORCH "Enable TorchScript value-net wrapper" OFF)
if (QUASAR_BUILD_TORCH)
  find_package(Torch QUIET)
  if (Torch_FOUND)
    message(STATUS "Found Torch: ${Torch_VERSION}")
    target_sources(quasar_engine PRIVATE src/nn/torchscript_value_net.cpp)
    target_compile_definitions(quasar_engine PRIVATE QUASAR_USE_TORCH)
    target_link_libraries(quasar_engine PUBLIC ${TORCH_LIBRARIES})
    target_include_directories(quasar_engine PUBLIC ${TORCH_INCLUDE_DIRS})
  else()
    message(WARNING "Torch not found; disabling QUASAR_BUILD_TORCH")
  endif()
endif()
