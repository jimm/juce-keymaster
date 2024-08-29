#include "cursor_test.h"
#include "test_helper.h"

void CursorTest::shutdown() {
  delete km;
}

void CursorTest::runTest() {
  test_init_empty();
  test_cursor();
}

void CursorTest::test_cursor() {
  beginTest("initialization");

  km = load_test_data(dev_mgr);
  c = km->cursor();

  // ==== init
  c->init();
  expect(c->set_list_index == 0);
  expect(c->song_index == 0);
  expect(c->patch_index == 0);
  expect(c->song()->name() == "Another Song");
  expect(c->patch()->name() == "Two Inputs Merging");

  beginTest("movement");

  // ==== next patch
  c->init();
  c->next_patch();
  expect(c->set_list_index == 0);
  expect(c->song_index == 0);
  expect(c->patch_index == 1);
  expect(c->song()->name() == "Another Song");
  expect(c->patch()->name() == "Split Into Two Outputs");

  // ==== next patch at end of song
  c->init();
  c->patch_index = 1;
  c->next_patch();
  expect(c->set_list_index == 0);
  expect(c->song_index == 1);
  expect(c->patch_index == 0);
  expect(c->song()->name() == "Song Without Explicit Patch");
  expect(c->patch()->name() == "Song Without Explicit Patch"); // created by default

  // ==== next patch at end of set list
  c->set_list_index = 1;        // Set List One
  c->song_index = 1;            // Another Song
  c->patch_index = 1;           // Split Into Two Outputs
  expect(c->set_list()->name() == "Set List One");
  expect(c->song()->name() == "Another Song");
  expect(c->patch()->name() == "Split Into Two Outputs");
  c->next_patch();
  expect(c->set_list_index == 1);
  expect(c->song_index == 1);
  expect(c->patch_index == 1);

  // ==== prev patch
  c->init();
  c->patch_index = 1;
  c->prev_patch();
  expect(c->set_list_index == 0);
  expect(c->song_index == 0);
  expect(c->patch_index == 0);

  // ==== prev patch start of song
  c->init();
  c->song_index = 1;
  c->prev_patch();
  expect(c->set_list_index == 0);
  expect(c->song_index == 0);
  expect(c->patch_index == 0);

  // ==== prev patch start of set list
  c->init();
  c->prev_patch();
  expect(c->set_list_index == 0);
  expect(c->song_index == 0);
  expect(c->patch_index == 0);

  // ==== next song
  c->init();
  c->next_song();
  expect(c->set_list_index == 0);
  expect(c->song_index == 1);
  expect(c->patch_index == 0);

  // ==== prev song
  c->init();
  c->song_index = 1;
  c->patch_index = 1;
  c->prev_song();
  expect(c->set_list_index == 0);
  expect(c->song_index == 0);
  expect(c->patch_index == 0);

  beginTest("has_{next,prev}_{song,patch} predicates");

  // ==== has next song true
  c->init();
  expect(c->has_next_song());

  // ==== has next song false
  c->init();
  c->song_index = c->set_list()->songs().size() - 1;
  expect(!c->has_next_song());

  // ==== has prev song true
  c->init();
  c->song_index = 1;
  expect(c->has_prev_song());

  // ==== has prev song false
  c->init();
  expect(!c->has_prev_song());

  // ==== has next patch true
  c->init();
  expect(c->has_next_patch());

  // ==== has next patch false
  c->init();
  c->song_index = c->set_list()->songs().size() - 1;
  c->patch_index = c->song()->patches().size() - 1;
  expect(!c->has_next_patch());

  // ==== has prev patch true
  c->init();
  c->patch_index = 1;
  expect(c->has_prev_patch());

  // ==== has prev patch false
  c->init();
  expect(!c->has_prev_patch());

  // ==== has next patch in song true
  c->init();
  expect(c->has_next_patch_in_song());

  // ==== has next patch in song false
  c->init();
  c->song_index = c->set_list()->songs().size() - 1;
  c->patch_index = c->song()->patches().size() - 1;
  expect(!c->has_next_patch_in_song());

  // ==== has prev patch in song true
  c->init();
  c->song_index = 1;
  c->patch_index = 1;
  expect(c->has_prev_patch_in_song());

  // ==== has prev patch in song false
  c->init();
  c->song_index = 1;
  expect(!c->has_prev_patch_in_song());

  beginTest("defaults");

  // ==== default set list is all songs
  c->init();
  expect(c->set_list() == km->all_songs());

  // ==== song
  c->init();
  expect(c->song() == km->all_songs()->songs()[0]);

  // ==== patch
  c->init();
  Song *s = c->song();
  expect(c->patch() == s->patches()[0]);

  beginTest("goto");

  // ==== goto with regex
  // ==== song
  c->init();
  c->goto_song("nother");
  s = c->song();
  expect(s != nullptr);
  expect(s->name() == "Another Song");

  
  // ==== song, no such song
  c->init();
  Song *before_song = c->song();
  expect(before_song != nullptr);

  c->goto_song("nosuch");
  s = c->song();
  expect(s == before_song);

  
  // ==== set list
  c->init();
  c->goto_set_list("two");
  SetList *sl = c->set_list();
  expect(sl != nullptr);
  expect(sl->name() == "Set List Two");

  
  // ==== set list, no such set list
  c->init();
  SetList *before_setlist = c->set_list();
  expect(before_setlist != nullptr);

  c->goto_set_list("nosuch");
  sl = c->set_list();
  expect(sl == before_setlist);
  
  // ==== goto with pointer
  // ==== song
  c->init();
  c->goto_set_list("two");

  // ==== in set list
    
  // ==== not in set list
      
  // ==== patch
  c->init();
  Song *song = c->song();

  // ==== in song
  Patch *patch = song->patches().getLast();
  expect(c->patch() != patch);
  c->goto_patch(patch);
  expect(c->song() == song);
  expect(c->patch() == patch);
    
  // ==== not in song
  c->init();
  Patch *before_patch = c->patch();

  c->goto_patch(nullptr);
  expect(c->song() == song);
  expect(c->patch() == before_patch);
}

void CursorTest::test_init_empty() {
  beginTest("init empty");

  km = new KeyMaster(dev_mgr, true);
  c = km->cursor();
  c->init();
  expect(c->set_list_index == 0);
  expect(c->song_index == -1);
  expect(c->patch_index == -1);
}
