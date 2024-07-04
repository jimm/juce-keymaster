#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "nameable.h"
#include "midi_device.h"

class MessageBlock : public DBObj, public Nameable {
public:
  MessageBlock(DBObjID id, const String &name);

  void from_string(const String &str);
  void from_editable_string(const String &str);

  String to_string();
  String to_editable_string();

  void send_to_all_outputs();
  void send_to(MidiOutputEntry::Ptr);

private:
  MidiBuffer midi_messages;

  void from_tokens(StringArray &tokens);
};
