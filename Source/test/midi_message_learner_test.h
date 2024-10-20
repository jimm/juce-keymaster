#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"

class MidiMessageLearnerTest : public UnitTest {
public:
  MidiMessageLearnerTest() : UnitTest("MidiMessageLearner Test", "km:mmlearner") {}

  void initialise() override;
  void runTest() override;

private:
  void test_want_midi_message();
  void test_capture();
  void test_max_messages();
};

static MidiMessageLearnerTest midi_message_learner_test;
