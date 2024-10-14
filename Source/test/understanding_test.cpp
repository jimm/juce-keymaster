#include "understanding_test.h"
#include "../km/consts.h"

// These tests help me understand how JUCE operates.

void UnderstandingTest::runTest() {
  // if message has one status and one data byte, can I construct it this way?
  auto msg = MidiMessage(PROGRAM_CHANGE, 42);
}
