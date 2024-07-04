#include <stdlib.h>
#include <string.h>
#include "consts.h"
#include "trigger.h"
#include "keymaster.h"

const MidiMessage EMPTY_MESSAGE = MidiMessage(ACTIVE_SENSE);

Trigger::Trigger(DBObjID id, const String &name, TriggerAction ta, MessageBlock *out_msg)
  : DBObj(id), Nameable(name),
    _trigger_input_identifier(""),
    _trigger_key_code(UNDEFINED),
    _trigger_message(ACTIVE_SENSE),
    _action(ta),
    _output_message(out_msg),
    _trigger_message_num_bytes(0)
{
}

// `message` might be EMPTY_MESSAGE, which will always be ignored.
void Trigger::set_trigger_message(MidiInput *input, MidiMessage message) {
  _trigger_input_identifier = input->getIdentifier();
  _trigger_message = message;
  _trigger_message_num_bytes = message.getRawDataSize();
}

bool Trigger::signal_message(MidiInput* source, const MidiMessage& message) {
  if (source->getIdentifier() != _trigger_input_identifier
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
  case TA_NEXT_SONG:
    km->next_song();
    break;
  case TA_PREV_SONG:
    km->prev_song();
    break;
  case TA_NEXT_PATCH:
    km->next_patch();
    break;
  case TA_PREV_PATCH:
    km->prev_patch();
    break;
  case TA_PANIC:
    km->panic(false);
    break;
  case TA_SUPER_PANIC:
    km->panic(true);
    break;
  case TA_TOGGLE_CLOCK:
    km->toggle_clock();
    break;
  case TA_MESSAGE:
    _output_message->send_to_all_outputs();
    break;
  }
}
