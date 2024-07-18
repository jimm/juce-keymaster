#include <stdlib.h>
#include <string.h>
#include "consts.h"
#include "trigger.h"
#include "keymaster.h"

const MidiMessage EMPTY_MESSAGE = MidiMessage(ACTIVE_SENSE);

Trigger::Trigger(DBObjID id, const String &name, TriggerAction ta, MessageBlock *out_msg)
  : DBObj(id), Nameable(name),
    _trigger_input(nullptr),
    _trigger_key_code(UNDEFINED),
    _trigger_message(ACTIVE_SENSE),
    _action(ta),
    _output_message(out_msg),
    _trigger_message_num_bytes(0)
{
}

void Trigger::set_trigger_key_code(int key_code) {
  if (_trigger_key_code != key_code) {
    _trigger_key_code = key_code;
    KeyMaster_instance()->changed();
  }
}

// To erase trigger message, make input == nullptr
// `message` might be EMPTY_MESSAGE, which will always be ignored.
//
// Reemember that MidiMessage channels go from 1-16
void Trigger::set_trigger_message(Input::Ptr input, MidiMessage message) {
  _trigger_input = input;
  _trigger_message = message;
  _trigger_message_num_bytes = message.getRawDataSize();
  KeyMaster_instance()->changed();
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

bool Trigger::signal_message(Input::Ptr input, const MidiMessage& message) {
  if (input != _trigger_input
      || !input->is_running()
      || message.getRawDataSize() != _trigger_message_num_bytes
      || memcmp(message.getRawData(), _trigger_message.getRawData(), _trigger_message_num_bytes) != 0)
    return false;

  perform_action();
  return true;
}

bool Trigger::signal_key(int key_code) {
  if (key_code != _trigger_key_code)
    return false;

  perform_action();
  return true;
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
