#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"

class DeviceManagerTest : public UnitTest {
public:
  DeviceManagerTest() : UnitTest("Device Manager Test", "km:device_manager") {}

  void runTest() override;

private:
  DeviceManager dev_mgr;

  void test_find_and_create();
  void test_different_device_identifiers();
  void clear_dev_mgr();
};

static DeviceManagerTest device_manager_test;
