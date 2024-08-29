#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"

class ControllerTest : public UnitTest {
public:
  ControllerTest() : UnitTest("Controller Test", "km:controller") {}

  void runTest() override;
};

static ControllerTest controller_test;
