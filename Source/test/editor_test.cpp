#include "editor_test.h"
#include "test_helper.h"
#include "../km/editor.h"
#include "../km/keymaster.h"

void EditorTest::reload() {
  km = load_test_data(dev_mgr);
  cursor = km->cursor();
  cursor->init();
  if (e != nullptr)
    delete e;
  e = new Editor(km);
}

void EditorTest::shutdown() {
  delete km;
  delete e;
}

void EditorTest::runTest() {
  test_remove_song_from_all_songs();
  test_remove_song_from_set_list();
}

void EditorTest::test_remove_song_from_all_songs() {
  // ================
  beginTest("all_songs: destroyed and cursor unchanged when not current song");
  reload();

  Song *current_song = cursor->song();
  expect(cursor->set_list() == km->all_songs()); // make sure we're on all songs
  Song *delete_me = km->all_songs()->songs()[1];

  e->remove_song_from_set_list(cursor->set_list(), delete_me);

  for (auto set_list : km->set_lists())
    for (auto song : set_list->songs())
      expect(song != delete_me, "song was not removed from a set list");
  expect(cursor->song() == current_song);

  // ================
  beginTest("all_songs: destroyed, cursor moved when current song");
  reload();

  current_song = cursor->song();
  delete_me = current_song;

  e->remove_song_from_set_list(cursor->set_list(), delete_me);

  for (auto set_list : km->set_lists())
    for (auto song : set_list->songs())
      expect(song != delete_me, "song was not removed from a set list");
  expect(cursor->song() != current_song);
}

void EditorTest::test_remove_song_from_set_list() {
  // ================
  beginTest("set list: removed and cursor unchanged when not current song");
  reload();

  cursor->set_list_index = 1;
  SetList *set_list = cursor->set_list();
  expect(set_list != km->all_songs()); // make sure we're not all songs
  Song *current_song = cursor->song();
  Song *delete_me = set_list->songs()[1];

  e->remove_song_from_set_list(set_list, delete_me);

  // make sure was removed
  for (auto song : set_list->songs())
    expect(song != delete_me, "song was not removed from a set list");

  // make sure still in all songs
  bool found = false;
  for (auto song : km->all_songs()->songs())
    if (song == delete_me)
      found = true;
  expect(found);

  expect(cursor->song() == current_song, "shouldn't have moved");

  // ================
  beginTest("set list: removed and cursor moved when current song");
  reload();

  cursor->set_list_index = 1;
  set_list = cursor->set_list();
  expect(set_list != km->all_songs(), "should not be on all songs"); // make sure we're not all songs
  current_song = cursor->song();
  delete_me = current_song;

  e->remove_song_from_set_list(set_list, delete_me);

  // make sure was removed
  for (auto song : set_list->songs())
    expect(song != delete_me, "song was not removed from a set list");

  // make sure still in all songs
  found = false;
  for (auto song : km->all_songs()->songs())
    if (song == delete_me)
      found = true;
  expect(found);

  expect(cursor->song() != current_song, "should have moved");
}
