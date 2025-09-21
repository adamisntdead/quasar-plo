#include <iostream>

int main() {
  try {
    return 0;
  } catch (...) {
    std::cerr << "Unhandled exception in tests" << std::endl;
    return 1;
  }
}

