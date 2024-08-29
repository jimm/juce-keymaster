#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"

class CursorTest : public UnitTest {
public:
  CursorTest() : UnitTest("Cursor Test", "km:cursor") {}

  void shutdown() override;
  void runTest() override;

private:
  DeviceManager dev_mgr;
  KeyMaster *km;
  Cursor *c;

  void test_init_empty();
  void test_cursor();
};

static CursorTest cursor_test;
