#include "editor_test.h"
#include "test_helper.h"
#include "../km/editor.h"
#include "../km/keymaster.h"

void EditorTest::reset() {
  km = KeyMaster_instance();
  cursor = km->cursor();
  cursor->init();
  if (e != nullptr)
    delete e;
  e = new Editor(km);
}

void EditorTest::reload() {
  if (km != nullptr)
    delete km;
  km = load_test_data(dev_mgr);
  reset();
}

void EditorTest::shutdown() {
  if (km != nullptr)
    delete km;
  if (e != nullptr)
    delete e;
}

void EditorTest::runTest() {
  test_remove_song_from_all_songs();
  test_remove_song_from_set_list();
  test_create();
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

void EditorTest::test_create() {
  beginTest("new conn when no song or patch yet");
  if (km != nullptr)
    delete km;
  km = new KeyMaster(dev_mgr);
  reset();

  // precondition checks
  expect(km == KeyMaster_instance());
  expect(km->all_songs() != nullptr);
  expect(km->all_songs()->songs().isEmpty());

  auto conn = e->create_connection(nullptr, nullptr);
  e->add_connection(nullptr, conn);

  expect(km->all_songs()->songs().size() == 1);

  auto song = km->cursor()->song();
  expect(song != nullptr);
  expect(!song->patches().isEmpty());

  auto patch = km->cursor()->patch();
  expect(patch == song->patches()[0]);
  expect(conn == patch->connections()[0]);
}
