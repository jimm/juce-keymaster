#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "nameable.h"
#include "output.h"

class MessageBlock : public DBObj, public Nameable, public MidiInputCallback {
public:
  MessageBlock(DBObjID id, const String &name);

  MidiMessageSequence &midi_messages() { return _midi_messages; }

  void from_hex_string(const String &str);

  String to_hex_string();
  String to_editable_hex_string();

  void send_to_all_outputs();
  void send_to(Output::Ptr);

  // This MidiInputCallback method is only called when parsing a block of
  // MIDI bytes. It will never be called by actual incoming MIDI data.
  void handleIncomingMidiMessage(MidiInput *source, const MidiMessage &message) override;

private:
  MidiMessageSequence _midi_messages;

  String to_hex(String message_separator);
};
