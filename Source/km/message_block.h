#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "nameable.h"
#include "midi_device.h"

class MessageBlock : public DBObj, public Nameable {
public:
  MessageBlock(DBObjID id, const String &name);

  MidiMessageSequence &midi_messages() { return _midi_messages; }

  void from_string(const String &str);
  void from_editable_string(const String &str);

  String to_hex_string();
  String to_editable_hex_string();

  void send_to_all_outputs();
  void send_to(MidiOutputEntry::Ptr);

private:
  MidiMessageSequence _midi_messages;

  void from_tokens(StringArray &tokens);
};
