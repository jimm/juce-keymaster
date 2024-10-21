#include "test_helper.h"
#include "trigger_test.h"
#include "../km/trigger.h"

class TestTrigger : public Trigger {
public:
  TestTrigger() : Trigger(UNDEFINED, "test", TriggerAction::NEXT_SONG, nullptr), was_triggered(false) { }

  bool was_triggered;

  void reset() {
    was_triggered = false;
    _trigger_timestamp = 0;
  }
  int64 ts() { return _trigger_timestamp; }
  void set_ts(int64 val) { _trigger_timestamp = val; }
  void set_now() { _trigger_timestamp = Time::currentTimeMillis(); }

protected:
  virtual void perform_action() override { was_triggered = true; }
};

void TriggerTest::runTest() {
  test_triggers();
  test_note_off();
  test_debouncing();
}

void TriggerTest::test_triggers() {
  auto t = TestTrigger();
  MidiMessage msg, tmsg;

  beginTest("triggers properly, sets ts when needed");

  // system common / realtime
  t.reset();
  msg = MidiMessage::midiStart();
  t.set_trigger_message(msg);
  t.signal_message(msg);
  expect(t.was_triggered);

  // program change
  t.reset();
  msg = MidiMessage::programChange(3, 42);
  t.set_trigger_message(msg);
  t.signal_message(msg);
  expect(t.was_triggered);

  // pitch bend
  t.reset();
  tmsg = MidiMessage::pitchWheel(3, 0x100);
  msg = MidiMessage::pitchWheel(3, 0x78);
  t.set_trigger_message(tmsg);
  t.signal_message(msg);
  expect(t.was_triggered);
  expect(t.ts() > 0);

  // sending another PB value that's "off-ish" --- low value -- resets
  t.was_triggered = false;
  msg = MidiMessage::pitchWheel(3, 8);
  t.signal_message(msg);
  expect(!t.was_triggered);
  expect(t.ts() == 0);

  // switch controller
  t.reset();
  msg = MidiMessage::controllerEvent(3, CC_SUSTAIN, 127);
  t.set_trigger_message(msg);
  t.signal_message(msg);
  expect(t.was_triggered);
  expect(t.ts() == 0);

  // bank select must be exact value
  t.reset();
  tmsg = MidiMessage::controllerEvent(3, CC_BANK_SELECT_LSB, 4);
  t.set_trigger_message(tmsg);
  msg = MidiMessage::controllerEvent(3, CC_BANK_SELECT_LSB, 64);
  t.signal_message(msg);
  expect(!t.was_triggered);
  expect(t.ts() == 0);

  msg = MidiMessage::controllerEvent(3, CC_BANK_SELECT_LSB, 4);
  t.signal_message(msg);
  expect(t.was_triggered);
  expect(t.ts() == 0);

  // continuous controller
  t.reset();
  tmsg = MidiMessage::controllerEvent(3, CC_PAN, 127);
  msg = MidiMessage::controllerEvent(3, CC_PAN, 64);
  t.set_trigger_message(tmsg);
  t.signal_message(msg);
  expect(t.was_triggered);
  expect(t.ts() > 0);
}

void TriggerTest::test_note_off() {
  auto t = TestTrigger();
  MidiMessage msg, tmsg;

  beginTest("note on/off");

  // note on
  t.reset();
  tmsg = MidiMessage::noteOn(3, 64, (uint8)80);
  t.set_trigger_message(tmsg);
  msg = MidiMessage::noteOn(3, 64, (uint8)90);
  t.signal_message(msg);
  expect(t.was_triggered);
  expect(t.ts() > 0);

  // reset when we see note off corresponding to note on
  t.was_triggered = false;
  msg = MidiMessage::noteOff(3, 64, (uint8)90);
  t.signal_message(msg);
  expect(!t.was_triggered);
  expect(t.ts() == 0);

  // note off (other channel message)
  t.reset();
  tmsg = MidiMessage::noteOff(3, 64, (uint8)80);
  t.set_trigger_message(tmsg);

  // do not trigger when we see note on with non-zero velocity
  msg = MidiMessage::noteOn(3, 64, (uint8)90);
  t.signal_message(msg);
  expect(!t.was_triggered);
  expect(t.ts() == 0);

  // trigger when we see note on with velocity 0 for same note
  msg = MidiMessage::noteOn(3, 64, (uint8)0);
  t.signal_message(msg);
  expect(t.was_triggered);
  expect(t.ts() > 0);
}

void TriggerTest::test_debouncing() {
  auto t = TestTrigger();
  MidiMessage msg, tmsg;

  beginTest("test debouncing");

  t.reset();
  tmsg = MidiMessage::controllerEvent(3, CC_PAN, (uint8)127);
  t.set_trigger_message(tmsg);

  msg = MidiMessage::controllerEvent(3, CC_PAN, (uint8)12);
  t.signal_message(msg);
  expect(t.was_triggered);
  auto ts = t.ts();
  expect(ts > 0);

  // too soon, not triggered but time stamp remains the same
  t.was_triggered = false;
  msg = MidiMessage::controllerEvent(3, CC_PAN, (uint8)12);
  t.signal_message(msg);
  expect(!t.was_triggered);
  expect(t.ts() == ts);

  // get a zero value, reset timestamp
  t.was_triggered = false;
  msg = MidiMessage::controllerEvent(3, CC_PAN, (uint8)0);
  t.signal_message(msg);
  expect(!t.was_triggered);
  expect(t.ts() == 0);
}
