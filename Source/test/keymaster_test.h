#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"

class KeyMasterTest : public UnitTest {
public:
  KeyMasterTest() : UnitTest("KeyMaster Test", "km:keymaster") {}

  void initialise() override;
  void shutdown() override;
  void runTest() override;

private:
  DeviceManager dev_mgr;
  KeyMaster *km;

  void test_all_songs_sorted();
  void test_inserted_song_sorts_case_insensitively();
};

static KeyMasterTest keymaster_test;
