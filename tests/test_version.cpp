#include "quasar/engine/version.h"
#include <cassert>
#include <iostream>

int main() {
  std::cout << "Quasar version: " << quasar::VERSION << std::endl;
  assert(quasar::VERSION[0] != '\0');
  return 0;
}

