#include <JuceHeader.h>
#include <juce_audio_basics/midi/juce_MidiDataConcatenator.h>
#include "consts.h"
#include "keymaster.h"
#include "message_block.h"
// #include "error.h"

#define NON_REALTIME_STATUS(b) ((b) >= NOTE_OFF && (b) <= EOX)

MessageBlock::MessageBlock(DBObjID id, const String &name)
  : DBObj(id), Nameable(name)
{
}

// Reads `str` which must consist of bytes encoded as two-digit hex strings
// separated by whitespace. Replaces the current messages.
void MessageBlock::from_hex_string(const String &str) {
  _midi_messages.clear();

  StringArray tokens;
  tokens.addTokens(str, false);

  uint8 data[str.length()];
  int i = 0;
  for (auto str : tokens)
    data[i++] = (uint8)str.getHexValue32();

  MidiDataConcatenator concatenator(1024);
  concatenator.pushMidiData((const void *)data, (int)str.length(), (double)0, (MidiInput *)nullptr, *this);

  KeyMaster_instance()->changed();
}

// Returns a string consisting of space-separated two-digit hex bytes.
String MessageBlock::to_hex_string() {
  return to_hex(" ");
}

// Returns a string consisting of space- and newline-delimited two-digit hex
// numbers. Each message is separated by a newline and each byte within the
// message is separated by a space.
String MessageBlock::to_editable_hex_string() {
  return to_hex("\n");
}

String MessageBlock::to_hex(String message_separator) {
  StringArray strs;
  for (auto &msg : _midi_messages)
    strs.add(String::toHexString(msg->message.getRawData(), msg->message.getRawDataSize()));
  return strs.joinIntoString(message_separator);
}

void MessageBlock::send_to_all_outputs() {
  for (auto &out : KeyMaster_instance()->device_manager().outputs())
    send_to(out);
}

void MessageBlock::send_to(Output::Ptr out) {
  for (auto &msg : _midi_messages)
    out->midi_out(msg->message);
}

// This MidiInputCallback method is only called when parsing a block of MIDI
// bytes. It will never be called by actual incoming MIDI data.
void MessageBlock::handleIncomingMidiMessage(MidiInput *_source, const MidiMessage &message) {
  _midi_messages.addEvent(message, 0);
}
