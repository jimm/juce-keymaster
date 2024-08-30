#include "keymaster_test.h"
#include "test_helper.h"

void KeyMasterTest::initialise() {
  km = load_test_data(dev_mgr);
}

void KeyMasterTest::shutdown() {
  delete km;
}

void KeyMasterTest::runTest() {
  test_all_songs_sorted();
  test_inserted_song_sorts_case_insensitively();
}

void KeyMasterTest::test_all_songs_sorted() {
  beginTest("all songs sorted");
  expect(km->all_songs()->songs()[0]->name() == "Another Song");
  expect(km->all_songs()->songs()[1]->name() == "Song Without Explicit Patch");
  expect(km->all_songs()->songs()[2]->name() == "This is a Song");
}

void KeyMasterTest::test_inserted_song_sorts_case_insensitively() {
  beginTest("inserted song sorts properly");
  Song *s = new Song(UNDEFINED_ID, "bees, bees!");
  km->all_songs()->add_song(s);

  expect(km->all_songs()->songs()[0]->name() == "Another Song");
  expect(km->all_songs()->songs()[1]->name() == "bees, bees!");
  expect(km->all_songs()->songs()[2]->name() == "Song Without Explicit Patch");
  expect(km->all_songs()->songs()[3]->name() == "This is a Song");
}
