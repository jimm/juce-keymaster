#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"

class EditorTest : public UnitTest {
public:
  EditorTest() : UnitTest("Editor Test", "km:editor") {}

  void shutdown() override;
  void runTest() override;

private:
  DeviceManager dev_mgr;
  KeyMaster *km;
  Cursor *cursor;

  void reload();
  void test_remove_song_from_all_songs();
  void test_remove_song_from_set_list();
};

static EditorTest editor_test;
