#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "nameable.h"
#include "output.h"

class MessageBlock : public DBObj, public Nameable {
public:
  MessageBlock(DBObjID id, const String &name);

  Array<MidiMessage> &midi_messages() { return _midi_messages; }

  void from_hex_string(const String &str);

  String to_hex_string();
  String to_editable_hex_string();

  void send_to_all_outputs();
  void send_to(Output::Ptr);

private:
  Array<MidiMessage> _midi_messages;

  String to_hex(String message_separator);
  MidiMessage sysex_from_bytes(uint8 *data, int start, int num_bytes);
};
