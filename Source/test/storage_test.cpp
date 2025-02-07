#include "test_helper.h"
#include "storage_test.h"
#include "../km/storage.h"
#include "../km/editor.h"

// These are the indices of songs in the all-songs list. Different than test
// data file ID order because the all-songs list is sorted by song name.
#define ANOTHER_INDEX 0
#define SONG_WITHOUT_INDEX 1
#define THIS_IS_A_SONG_INDEX 2

void StorageTest::initialise() {
  km = nullptr;
}

void StorageTest::shutdown() {
  if (km != nullptr)
    delete km;
}

void StorageTest::runTest() {
  test_load("", true);
  test_save();
}

void StorageTest::test_load(String data_file_path, bool call_begin_test) {
  MidiMessage m, expected;

  km = load_test_data(dev_mgr, data_file_path);

  // ==== load messages
  if (call_begin_test)
    beginTest("load messages");
  expect(km->messages().size() == 5); // includes 2 start/stop messages

  MessageBlock *msg = km->messages()[0];
  expect(msg->name() == "Tune Request");
  expect(mm_eq(m = msg->midi_messages()[0], expected = MidiMessage(0xf6)));

  msg = km->messages()[1];
  expect(msg->name() == "Multiple Note-Offs");
  expect(mm_eq(m = msg->midi_messages()[0], expected = MidiMessage::noteOff(JCH(0), 64, (uint8)64)));
  expect(mm_eq(m = msg->midi_messages()[1], expected = MidiMessage::noteOff(JCH(1), 64, (uint8)65)));
  expect(mm_eq(m = msg->midi_messages()[2], expected = MidiMessage::noteOff(JCH(2), 42, (uint8)127)));

  msg = km->messages()[4];
  expect(msg->name() == "All Volumes Up");
  for (int chan = 0; chan < 16; ++chan)
    expect(mm_eq(m = msg->midi_messages()[chan], expected = MidiMessage::controllerEvent(JCH(chan), 7, 127)));

  // ==== load triggers
  if (call_begin_test)
    beginTest("load triggers");
  Trigger *t;

  expect(km->triggers().size() == 7);

  // keys
  t = km->triggers()[0];
  expect(t->name() == "panic");
  expect(t->has_trigger_key_press());
  expect(t->trigger_key_press().getKeyCode() == 340);
  expect(mm_eq(m = t->trigger_message(), expected = EMPTY_MESSAGE));
  expect(t->output_message() == nullptr);
  expect(t->action() == TriggerAction::PANIC);

  t = km->triggers()[2];
  expect(t->name() == "next song");
  expect(mm_eq(m = t->trigger_message(), expected = MidiMessage(0xb0, 0x32, 0x7f)));
  expect(t->action() == TriggerAction::NEXT_SONG);
  expect(t->output_message() == nullptr);

  t = km->triggers().getLast();
  expect(t->name() == "send tune request message");
  expect(t->action() == TriggerAction::MESSAGE);
  expect(mm_eq(m = t->trigger_message(), expected = MidiMessage(0xb0, 0x36, 0x7f)));
  expect(t->output_message() != nullptr);
  expect(t->output_message()->name() == "Tune Request");

  // ==== load songs
  if (call_begin_test)
    beginTest("load songs");
  Array<Song *> &all_songs = km->all_songs()->songs();
  expect(all_songs.size() == 3);

  Song *s = all_songs[0];
  expect(s->name() == "Another Song");

  s = all_songs[1];
  expect(s->name() == "Song Without Explicit Patch");

  s = all_songs[2];
  expect(s->name() == "This is a Song");

  // ==== load notes
  if (call_begin_test)
    beginTest("load notes");
  s = all_songs[SONG_WITHOUT_INDEX];
  expect(s->notes() == "");

  s = all_songs[ANOTHER_INDEX];
  expect(s->notes() == "this song has note text\nthat spans multiple lines");

  // ==== load patches
  if (call_begin_test)
    beginTest("load patches");
  s = all_songs[THIS_IS_A_SONG_INDEX];
  expect(s->patches().size() == 2);

  Patch *p = s->patches()[0];
  expect(p->name() == "Vanilla Through, Filter Two's Sustain");

  // ==== load start and stop messages
  if (call_begin_test)
    beginTest("load start and stop messages");
  s = all_songs[ANOTHER_INDEX];
  p = s->patches().getLast();
  expect(p->start_message()->name() == "_start");
  expect(p->stop_message()->name() == "_stop");

  // ==== load connections
  if (call_begin_test)
    beginTest("load connections");
  s = all_songs[THIS_IS_A_SONG_INDEX]; // This is a Song
  p = s->patches()[0];          // Two Inputs Merging
  expect(p->connections().size() == 2);
  Connection *conn = p->connections()[0];
  expect(conn->input()->name() == "first input");
  expect(conn->input_chan() == CONNECTION_ALL_CHANNELS);
  expect(conn->output()->name() == "first output");
  expect(conn->output_chan() == CONNECTION_ALL_CHANNELS);

  s = all_songs[ANOTHER_INDEX];  // Another Song
  p = s->patches().getLast();        // Split Into Two OUtupts
  expect(p->connections().size() == 2);
  conn = p->connections()[0];
  expect(conn->input_chan() == 2);
  expect(conn->output_chan() == 3);

  MessageFilter &mf = conn->message_filter();
  expect(mf.note());

  // ==== load bank msb, lsb, and program
  if (call_begin_test)
    beginTest("load bank msb, lsb, and program");
  s = all_songs[THIS_IS_A_SONG_INDEX]; // This is a Song
  p = s->patches()[0];          // Vanilla Through
  expect(p->name() == "Vanilla Through, Filter Two's Sustain");
  conn = p->connections().getLast();

  expect(conn->program_bank_msb() == 3);
  expect(conn->program_bank_lsb() == 2);
  expect(conn->program_prog() == 12);

  // ==== load bank lsb only
  if (call_begin_test)
    beginTest("load bank lsb only");
  s = all_songs[THIS_IS_A_SONG_INDEX]; // This is a Song
  p = s->patches()[1];          // One Up One Oct...
  conn = p->connections().getLast();
  expect(conn->program_bank_msb() == UNDEFINED);
  expect(conn->program_bank_lsb() == 5);
  expect(conn->program_prog() == UNDEFINED);

  // ==== load xpose and velocity_curve
  if (call_begin_test)
    beginTest("load xpose and velocity_curve");
  s = all_songs[THIS_IS_A_SONG_INDEX];
  p = s->patches()[0];
  conn = p->connections()[0];
  expect(conn->xpose() == 0);

  p = s->patches().getLast();
  conn = p->connections()[0];
  expect(conn->xpose() == 12);
  expect(conn->velocity_curve() == nullptr);
  conn = p->connections().getLast();
  expect(conn->xpose() == -12);
  expect(conn->velocity_curve()->name() == "Test Curve");

  // ==== load zone
  if (call_begin_test)
    beginTest("load zone");
  s = all_songs[THIS_IS_A_SONG_INDEX];
  p = s->patches()[0];
  conn = p->connections()[0];
  expect(conn->zone_low() == 0);
  expect(conn->zone_high() == 127);

  s = all_songs[ANOTHER_INDEX];  // Another Song
  p = s->patches()[1];
  conn = p->connections()[0];
  expect(conn->zone_low() == 0);
  expect(conn->zone_high() == 63);

  conn = p->connections()[1];
  expect(conn->zone_low() == 64);
  expect(conn->zone_high() == 127);

  // ==== load controller mappings
  if (call_begin_test)
    beginTest("load controller mappings");
  s = all_songs[THIS_IS_A_SONG_INDEX];
  p = s->patches()[0];
  conn = p->connections().getLast();
  expect(conn->cc_map(64) != nullptr);
  expect(conn->cc_map(64)->filtered() == true);

  p = s->patches()[1];
  conn = p->connections()[0];
  Controller *cc = conn->cc_map(7);
  expect(cc != nullptr);
  expect(cc->translated_cc_num() == 10);
  expect(cc->filtered() == false);
  expect(cc->pass_through_0() == true);
  expect(cc->pass_through_127() == false);
  expect(cc->min_in() == 1);
  expect(cc->max_in() == 120);
  expect(cc->min_out() == 40);
  expect(cc->max_out() == 50);

  // ==== load set list
  if (call_begin_test)
    beginTest("load set list");
  expect(km->set_lists().size() == 3);

  SetList *sl = km->set_lists()[1]; // first user-defined set list
  expect(sl->name() == "Set List One");
  expect(sl->songs().size() == 2);
  expect(sl->songs()[0] == all_songs[THIS_IS_A_SONG_INDEX]);
  expect(sl->songs().getLast() == all_songs[ANOTHER_INDEX]);

  sl = km->set_lists()[2];       // second user-defined set list
  expect(sl->name() == "Set List Two");
  expect(sl->songs().size() == 2);
  expect(sl->songs()[0] == all_songs[ANOTHER_INDEX]);
  expect(sl->songs().getLast() == all_songs[THIS_IS_A_SONG_INDEX]);

  // ==== load auto patch
  if (call_begin_test)
    beginTest("load auto patch");
  s = all_songs[SONG_WITHOUT_INDEX];
  expect(s->patches().size() == 1);
  p = s->patches()[0];
  expect(p->name() == s->name());
  expect(p->connections().size() == 1);
  expect(p->connections()[0]->input()->name() == "first input");
  expect(p->connections()[0]->output()->name() == "first output");
}

void StorageTest::test_save() {
  beginTest("save (after successful save, calls test_load)");
  File save_file("/tmp/km_save_test.kmst");
  Storage saver(dev_mgr, save_file);
  saver.save(km);
  expect(saver.has_error() == false);
  test_load(save_file.getFullPathName(), false);
  save_file.deleteFile();
}
