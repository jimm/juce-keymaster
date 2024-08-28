#include "cursor_test.h"
#include "test_helper.h"

void CursorTest::initialise() {
  km = load_test_data(dev_mgr);
  c = km->cursor();
}

void CursorTest::shutdown() {
  delete km;
}

void CursorTest::runTest() {
  test_init_empty();
  // FIXME really fix storage loading
  // test_cursor();
}

void CursorTest::test_init_empty() {
  delete km;
  km = new KeyMaster(dev_mgr, true);
  c = km->cursor();
  c->init();
  expect(c->set_list_index == 0);
  expect(c->song_index == -1);
  expect(c->patch_index == -1);
}

void CursorTest::test_cursor() {
  c->init();

// ================ initialization

  // ==== init
  expect(c->set_list_index == 0);
  expect(c->song_index == 0);
  expect(c->patch_index == 0);

// ================ movement

// ==== next patch
  c->next_patch();
  expect(c->set_list_index == 0);
  expect(c->song_index == 0);
  expect(c->patch_index == 1);

// ==== next patch at end of song
  c->patch_index = 1;
  c->next_patch();
  expect(c->set_list_index == 0);
  expect(c->song_index == 1);
  expect(c->patch_index == 0);

// ==== next patch at end of set list
  c->set_list_index = 1;        // Set List One
  c->song_index = 1;            // Another Song
  c->patch_index = 1;           // Split Into Two Outputs
  c->next_patch();
  expect(c->set_list_index == 1);
  expect(c->song_index == 1);
  expect(c->patch_index == 1);

// ==== prev patch
  c->patch_index = 1;
  c->prev_patch();
  expect(c->set_list_index == 0);
  expect(c->song_index == 0);
  expect(c->patch_index == 0);

// ==== prev patch start of song
  c->song_index = 1;
  c->prev_patch();
  expect(c->set_list_index == 0);
  expect(c->song_index == 0);
  expect(c->patch_index == 0);

// ==== prev patch start of set list
  c->prev_patch();
  expect(c->set_list_index == 0);
  expect(c->song_index == 0);
  expect(c->patch_index == 0);

// ==== next song
  c->next_song();
  expect(c->set_list_index == 0);
  expect(c->song_index == 1);
  expect(c->patch_index == 0);

// ==== prev song
  c->song_index = 1;
  c->patch_index = 1;
  c->prev_song();
  expect(c->set_list_index == 0);
  expect(c->song_index == 0);
  expect(c->patch_index == 0);

// ================ has_{next,prev}_{song,patch} predicates

// ==== has next song true
  expect(c->has_next_song());

// ==== has next song false
  c->song_index = c->set_list()->songs().size() - 1;
  expect(!c->has_next_song());

// ==== has prev song true
  c->song_index = 1;
  expect(c->has_prev_song());

// ==== has prev song false
  expect(!c->has_prev_song());

// ==== has next patch true
  expect(c->has_next_patch());

// ==== has next patch false
  c->song_index = c->set_list()->songs().size() - 1;
  c->patch_index = c->song()->patches().size() - 1;
  expect(!c->has_next_patch());

// ==== has prev patch true
  c->patch_index = 1;
  expect(c->has_prev_patch());

// ==== has prev patch false
  expect(!c->has_prev_patch());

// ==== has next patch in song true
  expect(c->has_next_patch_in_song());

// ==== has next patch in song false
  c->song_index = c->set_list()->songs().size() - 1;
  c->patch_index = c->song()->patches().size() - 1;
  expect(!c->has_next_patch_in_song());

// ==== has prev patch in song true
  c->song_index = 1;
  c->patch_index = 1;
  expect(c->has_prev_patch_in_song());

// ==== has prev patch in song false
  c->song_index = 1;
  expect(!c->has_prev_patch_in_song());

// ================ defaults

// ==== default set list is all songs
  expect(c->set_list() == km->all_songs());

// ==== song
  expect(c->song() == km->all_songs()->songs()[0]);

// ==== patch
  Song *s = c->song();
  expect(c->patch() == s->patches()[0]);

// ================ goto

// ==== goto with regex
// ==== song
  c->goto_song("nother");
  s = c->song();
  expect(s != nullptr);
  expect(s->name() == "Another Song");

  
// ==== song, no such song
  Song *before_song = c->song();
  expect(before_song != nullptr);

  c->goto_song("nosuch");
  s = c->song();
  expect(s == before_song);

  
// ==== set list
  c->goto_set_list("two");
  SetList *sl = c->set_list();
  expect(sl != nullptr);
  expect(sl->name() == "Set List Two");

  
// ==== set list, no such set list
  SetList *before_setlist = c->set_list();
  expect(before_setlist != nullptr);

  c->goto_set_list("nosuch");
  sl = c->set_list();
  expect(sl == before_setlist);
  
// ==== goto with pointer
// ==== song
  c->goto_set_list("two");

// ==== in set list
    
// ==== not in set list
      
// ==== patch
  Song *song = c->song();

// ==== in song
  Patch *patch = song->patches().getLast();
  expect(c->patch() != patch);
  c->goto_patch(patch);
  expect(c->song() == song);
  expect(c->patch() == patch);
    
// ==== not in song
  Patch *before_patch = c->patch();

  c->goto_patch(nullptr);
  expect(c->song() == song);
  expect(c->patch() == before_patch);
      
  delete km;
}
