#include <JuceHeader.h>
#include <juce_audio_basics/midi/juce_MidiDataConcatenator.h>
#include "consts.h"
#include "keymaster.h"
#include "message_block.h"
#include "utils.h"

int CHANNEL_MESSAGE_LENGTHS[] = {
  3, 3, 3, 3, 2, 2, 3           // note off - pitch bend
};

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

  int num_bytes = tokens.size();
  uint8 *data = (uint8 *)malloc(size_t(num_bytes));
  int i = 0;
  for (auto s : tokens) {
    data[i++] = (uint8)s.getHexValue32();
    // FIXME if value is > 255 that's an error
  }

  for (i = 0; i < num_bytes; ++i) {
    uint8 byte = data[i];
    uint8 b2, b3;
    if (is_realtime(byte))
      _midi_messages.add(MidiMessage(byte));
    else if (is_channel(byte)) {
      switch (byte & 0xf0) {
      case NOTE_OFF: case NOTE_ON: case POLY_PRESSURE: case CONTROLLER: case PITCH_BEND:
        b2 = data[++i];
        b3 = data[++i];
        _midi_messages.add(MidiMessage(byte, b2, b3));
        break;
      default:
        b2 = data[++i];
        _midi_messages.add(MidiMessage(byte, b2));
        break;
      }
    }
    else {
      switch (byte) {
      case SONG_POINTER:
        b2 = data[++i];
        b3 = data[++i];
        _midi_messages.add(MidiMessage(byte, b2, b3));
        break;
      case SONG_SELECT:
        b2 = data[++i];
        _midi_messages.add(MidiMessage(byte, b2));
        break;
      case TUNE_REQUEST:
        _midi_messages.add(MidiMessage(byte));
        break;
      case SYSEX:
        MidiMessage sysex = sysex_from_bytes(data, i, num_bytes);
        _midi_messages.add(sysex);
        i += sysex.getSysExDataSize();
        break;
      }
    }
    // FIXME if i >= num_bytes that's an error
  }

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
    strs.add(String::toHexString(msg.getRawData(), msg.getRawDataSize()));
  return strs.joinIntoString(message_separator);
}

void MessageBlock::send_to_all_outputs() {
  for (auto &out : KeyMaster_instance()->device_manager().outputs())
    send_to(out);
}

void MessageBlock::send_to(Output::Ptr out) {
  for (auto &msg : _midi_messages)
    out->midi_out(msg);
}

MidiMessage MessageBlock::sysex_from_bytes(uint8 *data, int start, int num_bytes) {
  for (int i = start + 1; i < num_bytes; ++i) {
    if (data[i] == EOX || is_channel(data[i]))
      return MidiMessage::createSysExMessage(data + start + 1, i - start - 1);
  }
  return MidiMessage::createSysExMessage(data, 0);
}
