#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "quasar/engine/version.h"
#include "quasar/engine/public_state.h"

namespace py = pybind11;
using namespace quasar;

PYBIND11_MODULE(quasar_engine_py, m) {
  m.doc() = "QuasarPLO C++ engine bindings (minimal)";

  m.def("version", []() { return VERSION; }, "Engine semantic version");

  py::class_<PublicState>(m, "PublicState")
      .def(py::init<>())
      .def_readwrite("player_to_act", &PublicState::player_to_act)
      .def_readwrite("street", &PublicState::street)
      .def_readwrite("board", &PublicState::board)
      .def_readwrite("pot", &PublicState::pot)
      .def_readwrite("stacks", &PublicState::stacks)
      .def("to_string", &PublicState::to_string);
}

