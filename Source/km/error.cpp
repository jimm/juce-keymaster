#include <JuceHeader.h>
#include <iostream>
#include "error.h"

void error_message(const String &msg) {
  std::cerr << msg << "\n";
}
