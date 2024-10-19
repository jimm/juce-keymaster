#include <stdlib.h>
#include <string.h>
#include "consts.h"
#include "trigger.h"
#include "keymaster.h"
#include "utils.h"

Trigger::Trigger(DBObjID id, const String &name, TriggerAction ta, MessageBlock *out_msg)
  : DBObj(id), Nameable(name),
    _trigger_message(EMPTY_MESSAGE),
    _action(ta),
    _output_message(out_msg)
{
}

void Trigger::set_trigger_key_press(const KeyPress key_press) {
  if (_trigger_key_press != key_press) {
    _trigger_key_press = key_press;
    KeyMaster_instance()->changed();
  }
}

// To erase trigger message, make input == nullptr
// `message` might be EMPTY_MESSAGE, which will always be ignored.
//
// Reemember that MidiMessage channels go from 1-16
void Trigger::set_trigger_message(MidiMessage message) {
  if (!mm_equal(_trigger_message, message)) { // TODO better comparison for note-style messages
    _trigger_message = message;
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

bool Trigger::signal_message(const MidiMessage& message) {
  if (!mm_equal(message, _trigger_message)) // TODO better comparisons for note-style messages
    return false;

  perform_action();
  return true;
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
