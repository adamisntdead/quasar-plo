#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "quasar/engine/version.h"
#include "quasar/engine/public_state.h"
#include "quasar/engine/plo_legal.h"
#include "quasar/engine/json.h"
#include "quasar/engine/discretize.h"

namespace py = pybind11;
using namespace quasar;

PYBIND11_MODULE(quasar_engine_py, m) {
  m.doc() = "QuasarPLO C++ engine bindings (minimal)";

  m.def("version", []() { return VERSION; }, "Engine semantic version");

  py::class_<PublicState>(m, "PublicState")
      .def(py::init<>())
      .def_readwrite("num_players", &PublicState::num_players)
      .def_readwrite("player_to_act", &PublicState::player_to_act)
      .def_readwrite("button", &PublicState::button)
      .def_readwrite("street", &PublicState::street)
      .def_readwrite("board", &PublicState::board)
      .def_readwrite("sb", &PublicState::sb)
      .def_readwrite("bb", &PublicState::bb)
      .def_readwrite("ante", &PublicState::ante)
      .def_readwrite("stacks", &PublicState::stacks)
      .def_readwrite("committed_total", &PublicState::committed_total)
      .def_readwrite("committed_on_street", &PublicState::committed_on_street)
      .def_readwrite("last_raise_size", &PublicState::last_raise_size)
      .def("to_string", &PublicState::to_string)
      .def("pot_total", &PublicState::pot_total)
      .def("current_bet_to_call", &PublicState::current_bet_to_call)
      .def("amount_to_call", &PublicState::amount_to_call);

  m.def("solve_one_move_json", [](const std::string& json) {
    PublicState s;
    parse_public_state_from_json(json, s);
    auto la = compute_legal_actions(s);
    DiscretizationConfig cfg;
    auto discrete = discretize_actions(s, la, cfg);
    return assemble_response_json(la, discrete);
  }, "Parse a spot JSON and return a JSON summary of legal actions and a uniform discrete policy");
}
