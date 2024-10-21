#include <stdlib.h>
#include <string.h>
#include "consts.h"
#include "trigger.h"
#include "keymaster.h"
#include "utils.h"

#define MIN_LSB_VALUE 10
#define DEBOUNCE_MSECS 100

Trigger::Trigger(DBObjID id, const String &name, TriggerAction ta, MessageBlock *out_msg)
  : DBObj(id), Nameable(name),
    _trigger_message(EMPTY_MESSAGE),
    _action(ta),
    _output_message(out_msg),
    _trigger_timestamp(0)
{
}

void Trigger::set_trigger_key_press(const KeyPress key_press) {
  if (_trigger_key_press != key_press) {
    _trigger_key_press = key_press;
    KeyMaster_instance()->changed();
  }
}

// `message` can be EMPTY_MESSAGE, which will always be ignored.
void Trigger::set_trigger_message(MidiMessage message) {
  if (!mm_equal(_trigger_message, message)) { // TODO better comparison for note-style messages
    _trigger_message = message;
    _trigger_timestamp = 0;
    KeyMaster_instance()->changed();
  }
}

void Trigger::set_action(TriggerAction action) {
  if (_action != action) {
    _action = action;
    KeyMaster_instance()->changed();
  }
}

void Trigger::set_output_message(MessageBlock *msg) {
  if (_output_message != msg) {
    _output_message = msg;
    KeyMaster_instance()->changed();
  }
}

bool Trigger::too_soon() {
  return _trigger_timestamp > 0 && (Time::currentTimeMillis() - _trigger_timestamp) < DEBOUNCE_MSECS;
}

// Non-note messages match on status byte. PC matches on program number.
// Bank select MSB/LSB matches on value. Pitch bend matches on non-zero and
// reset on non-"low" pb values. Note messages match on status byte and note
// and non-zero velocity and reset on either zero velocity or timeout.
void Trigger::signal_message(const MidiMessage& message) {
  const uint8 *tdata = _trigger_message.getRawData();
  const uint8 *mdata = message.getRawData();
  uint8 status = mdata[0];

  // end debouncing if note on trigger, note off message (note on message
  // handled below)
  if (_trigger_message.isNoteOn() && message.isNoteOff(true)
      && _trigger_message.getChannel() == message.getChannel()
      && _trigger_message.getNoteNumber() == message.getNoteNumber()) {
    // got a note off that corresponds to our note on trigger message
    _trigger_timestamp = 0;
    return;
  }

  // perform action if note off trigger, note on message with velocity zero
  // (note off message handled below)
  if (_trigger_message.isNoteOff() && message.isNoteOn(true)
      && mdata[2] == 0
      && _trigger_message.getChannel() == message.getChannel()
      && _trigger_message.getNoteNumber() == message.getNoteNumber()) {
    // got a note on with velocity 0 that corresponds to our note off trigger message
    _trigger_timestamp = Time::currentTimeMillis();
    perform_action();
    return;
  }

  // return if statuses aren't the same
  if (status != tdata[0])
    return;

  // System common and system realtime messages
  if (status >= 0xF0) {
    perform_action();
    return;
  }

  // Channel message channels must match
  if ((status & 0x0f) != (tdata[0] & 0x0f))
    return;

  int high_nibble = status & 0xf0;
  uint8 tdata1 = tdata[1];
  uint8 mdata1 = mdata[1];

  // Program change
  if (high_nibble == PROGRAM_CHANGE) {
    // no debouncing necessary
    if (mdata1 == tdata1)
      perform_action();
    return;
  }

  // Pitch bend
  if (high_nibble == PITCH_BEND) {
    if (mdata[2] == 0 && mdata1 < MIN_LSB_VALUE) {
      _trigger_timestamp = 0;
      return;
    }
    if (too_soon())
      return;

    _trigger_timestamp = Time::currentTimeMillis();
    perform_action();
    return;
  }

  // Controller
  if (high_nibble == CONTROLLER) {
    if (mdata1 >= 64 && mdata1 <= 97) { // momentary switches
      // no debouncing necessary
      if (mdata[2] > 0)
        perform_action();
      return;
    }
    if (mdata1 == CC_BANK_SELECT_MSB || mdata1 == CC_BANK_SELECT_LSB) { // bank MSB/LSB
      // no debouncing necessary
      if (mdata[2] == tdata[2])
        perform_action();
      return;
    }
  }

  // Other channel messages that take a second value
  if (mdata[2] == 0) {
    // Got a zero value. Reset trigger timestamp without triggering.
    _trigger_timestamp = 0;
    return;
  }

  // debounce check
  if (too_soon())
    return;

  _trigger_timestamp = Time::currentTimeMillis();
  perform_action();
}

bool Trigger::keyPressed(const KeyPress &key, Component *) {
  if (_trigger_key_press == key)
    perform_action();
  return false;                 // pass the key through
}

void Trigger::perform_action() {
  KeyMaster *km = KeyMaster_instance();

  switch (_action) {
  case TriggerAction::NEXT_SONG:
    km->next_song();
    break;
  case TriggerAction::PREV_SONG:
    km->prev_song();
    break;
  case TriggerAction::NEXT_PATCH:
    km->next_patch();
    break;
  case TriggerAction::PREV_PATCH:
    km->prev_patch();
    break;
  case TriggerAction::PANIC:
    km->panic(false);
    break;
  case TriggerAction::SUPER_PANIC:
    km->panic(true);
    break;
  case TriggerAction::TOGGLE_CLOCK:
    km->toggle_clock();
    break;
  case TriggerAction::MESSAGE:
    _output_message->send_to_all_outputs();
    break;
  }
}
