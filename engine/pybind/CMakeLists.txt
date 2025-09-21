pybind11_add_module(quasar_engine_py
  module.cpp
)

target_link_libraries(quasar_engine_py PRIVATE quasar_engine)

