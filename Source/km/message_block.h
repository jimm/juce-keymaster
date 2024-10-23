#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "nameable.h"
#include "output.h"

class MessageBlock : public DBObj, public Nameable {
public:
  MessageBlock(DBObjID id, const String &name);

  Array<MidiMessage> &midi_messages() { return _midi_messages; }
  void clear() { _midi_messages.clear(); }

  void from_midi_messages(const Array<MidiMessage> &arr) { _midi_messages = arr; }
  void from_hex_string(const String &str);

  String to_hex_string();
  String to_editable_hex_string();

  void send_to_all_outputs();
  void send_to(Output::Ptr);

private:
  Array<MidiMessage> _midi_messages;

  MidiMessage sysex_from_bytes(uint8 *data, int start, int num_bytes);
};
