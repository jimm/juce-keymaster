#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"

class Editor;

class EditorTest : public UnitTest {
public:
  EditorTest() : UnitTest("Editor Test", "km:editor"), e(nullptr) {}

  void shutdown() override;
  void runTest() override;

private:
  DeviceManager dev_mgr;
  KeyMaster *km;
  Cursor *cursor;
  Editor *e;

  void reload();
  void test_remove_song_from_all_songs();
  void test_remove_song_from_set_list();
};

static EditorTest editor_test;