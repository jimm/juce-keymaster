#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"

class UnderstandingTest : public UnitTest {
public:
  UnderstandingTest() : UnitTest("Understanding Test", "km:understanding") {}

  void runTest() override;
};

static UnderstandingTest understanding_test;
