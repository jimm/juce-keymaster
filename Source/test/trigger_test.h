#pragma once

#include <JuceHeader.h>

class TriggerTest : public UnitTest {
public:
  TriggerTest() : UnitTest("Trigger Test", "km:trigger") {}

  void runTest() override;

private:
  void test_triggers();
  void test_note_off();
  void test_debouncing();
};

static TriggerTest trigger_test;
