#pragma once

#include <JuceHeader.h>
#include "output.h"

struct InstrumentProgramChange {
  Output::Ptr output;
  int channel { 1 };  // 1-16
  int bank_msb;       // UNDEFINED if not set
  int bank_lsb;       // UNDEFINED if not set
  int prog;           // UNDEFINED if not set

  InstrumentProgramChange();
  void send();
};
