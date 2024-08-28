#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"

class StorageTest : public UnitTest {
public:
  StorageTest() : UnitTest("Storage Test", "km") {}

  void initialise() override;
  void shutdown() override;
  void runTest() override;

private:
  DeviceManager dev_mgr;
  KeyMaster *km;

  void test_load(String data_file_path);
  void test_save();
};

static StorageTest storage_test;
