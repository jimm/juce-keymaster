#include <JuceHeader.h>
#include "consts.h"
#include "keymaster.h"
#include "message_block.h"
#include "formatter.h"
#include "utils.h"

int CHANNEL_MESSAGE_LENGTHS[] = {
  3, 3, 3, 3, 2, 2, 3           // note off - pitch bend
};

MessageBlock::MessageBlock(DBObjID id, const String &name)
  : DBObj(id), Nameable(name)
{
}

// Reads `str` which must consist of bytes encoded as two-digit hex strings
// separated by whitespace. Replaces the current messages and returns
// Result::ok(). If there is an error the messages will not be replaced and
// Result::fail() will be returned with an error message.
Result MessageBlock::from_hex_string(const String &str) {
  Array<MidiMessage> msgs;

  StringArray tokens;
  tokens.addTokens(str, false);

  int num_bytes = tokens.size();
  uint8 *data = (uint8 *)malloc(size_t(num_bytes));
  int i = 0;
  for (auto s : tokens) {
    int val = s.getHexValue32();
    if (val < 0 || val > 255)
      return Result::fail(String::formatted("byte %d is not between 0 (00) and 255 (ff)", i + 1));
    data[i++] = (uint8)val;
  }

  for (i = 0; i < num_bytes; ++i) {
    uint8 byte = data[i];
    uint8 b2, b3;
    if (is_realtime(byte))
      msgs.add(MidiMessage(byte));
    else if (is_channel(byte)) {
      switch (byte & 0xf0) {
      case NOTE_OFF: case NOTE_ON: case POLY_PRESSURE: case CONTROLLER: case PITCH_BEND:
        b2 = data[++i];
        b3 = data[++i];
        if (b2 >= 0x80 || b3 >= 0x80)
          return Result::fail(String::formatted("the two bytes starting at position %d must both be <= 127 (0x7f)", i - 1));
        msgs.add(MidiMessage(byte, b2, b3));
        break;
      default:
        b2 = data[++i];
        if (b2 >= 0x80)
          return Result::fail(String::formatted("byte %d must be <= 127 (0x7f)", i));
        msgs.add(MidiMessage(byte, b2));
        break;
      }
    }
    else {
      switch (byte) {
      case MTC_QFRAME: case SONG_SELECT:
        b2 = data[++i];
        if (b2 >= 0x80)
          return Result::fail(String::formatted("byte %d must be <= 127 (0x7f)", i));
        msgs.add(MidiMessage(byte, b2));
        break;
      case SONG_POINTER:
        b2 = data[++i];
        b3 = data[++i];
        if (b2 >= 0x80 || b3 >= 0x80)
          return Result::fail(String::formatted("the two bytes starting at position %d must both be <= 127 (0x7f)", i - 1));
        msgs.add(MidiMessage(byte, b2, b3));
        break;
      case TUNE_REQUEST:
        msgs.add(MidiMessage(byte));
        break;
      case SYSEX:
        MidiMessage sysex = sysex_from_bytes(data, i, num_bytes);
        msgs.add(sysex);
        i += sysex.getSysExDataSize() + 1;
        break;
      // FIXME this won't compile and I don't know why.
      // "Cannot jump from switch statement to this case label"
      //
      // default:
      //   return Result::fail(String::formatted("byte %d is not a legal status byte", i + 1));
      }
    }
    if (i >= num_bytes)
      return Result::fail("the last message was incomplete (not enough bytes)");
  }

  // No errors were thrown so we can copy the messages.
  _midi_messages = msgs;

  KeyMaster_instance()->changed();
  return Result::ok();
}

// Returns a string consisting of space-separated two-digit hex bytes.
String MessageBlock::to_hex_string() {
  return to_hex(_midi_messages, " ");
}

// Returns a string consisting of space- and newline-delimited two-digit hex
// numbers. Each message is separated by a newline and each byte within the
// message is separated by a space.
String MessageBlock::to_editable_hex_string() {
  return to_hex(_midi_messages, "\n");
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
