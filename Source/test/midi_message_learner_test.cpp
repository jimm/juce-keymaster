#include "test_helper.h"
#include "midi_message_learner_test.h"
#include "../km/consts.h"
#include "../km/midi_message_learner.h"

void MidiMessageLearnerTest::initialise() {
  set_KeyMaster_instance(nullptr);
}

void MidiMessageLearnerTest::runTest() {
  test_want_midi_message();
  test_capture();
  test_max_messages();
}

void MidiMessageLearnerTest::test_want_midi_message() {
  auto mml = MidiMessageLearner(false);
  uint8 sysex_data[1] = { 42 };
  
  beginTest("test various flags");

  expect(!mml.want_midi_message(MidiMessage::midiClock()));
  expect(!mml.want_midi_message(MidiMessage(ACTIVE_SENSE)));
  expect(!mml.want_midi_message(MidiMessage::createSysExMessage(sysex_data, 1)));

  expect(mml.want_midi_message(MidiMessage::noteOn(3, 64, (uint8)126)));
  expect(mml.want_midi_message(MidiMessage(SONG_POINTER)));
  expect(mml.want_midi_message(MidiMessage(START)));

  mml.reset(true, true, true);
  expect(mml.want_midi_message(MidiMessage::midiClock()));
  expect(mml.want_midi_message(MidiMessage(ACTIVE_SENSE)));
  expect(mml.want_midi_message(MidiMessage::createSysExMessage(sysex_data, 1)));
}

void MidiMessageLearnerTest::test_capture() {
  auto mml = MidiMessageLearner(false);
  uint8 sysex_data[1] = { 42 };
  auto sysex_message = MidiMessage::createSysExMessage(sysex_data, 1);
  MidiMessage msg, expected;

  beginTest("does not capture if not started");
  mml.reset();
  expect(!mml.is_learning());
  mml.learn_midi_message(MidiMessage::noteOn(3, 64, (uint8)126));
  expect(mml.midi_messages().size() == 0);

  beginTest("captures messages until stop, ignores others");
  mml.reset();
  mml.start_learning();
  expect(mml.is_learning());
  mml.learn_midi_message(MidiMessage::noteOn(3, 64, (uint8)126));
  mml.learn_midi_message(sysex_message);
  mml.learn_midi_message(MidiMessage::noteOff(3, 64, (uint8)126));
  mml.stop_learning();
  expect(!mml.is_learning());
  expect(mml.midi_messages().size() == 2);
  expect(mm_eq(msg = mml.midi_messages()[0], expected = MidiMessage::noteOn(3, 64, (uint8)126)));
  expect(mm_eq(msg = mml.midi_messages()[1], expected = MidiMessage::noteOff(3, 64, (uint8)126)));

  // Capture sysex if requested
  mml.reset(true);              // allow sysex
  mml.start_learning();
  mml.learn_midi_message(sysex_message); // filtered
  mml.stop_learning();
  expect(mml.midi_messages().size() == 1);
  expect(mm_eq(msg = mml.midi_messages()[0], expected = sysex_message));
}

void MidiMessageLearnerTest::test_max_messages() {
  auto mml = MidiMessageLearner(false);

  beginTest("learner max messages");
  mml.reset();
  mml.start_learning(1);
  mml.learn_midi_message(MidiMessage::noteOn(3, 64, (uint8)126));
  expect(!mml.is_learning());

  beginTest("learner max messages callback");
  mml.reset();
  int num_times_called = 0;
  mml.start_learning(1, [&num_times_called] () { ++num_times_called; });
  expect(mml.is_learning());
  mml.learn_midi_message(MidiMessage::noteOn(3, 64, (uint8)126));
  expect(!mml.is_learning());
  expect(num_times_called == 1);
}
