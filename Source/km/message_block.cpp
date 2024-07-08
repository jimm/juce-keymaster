#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include "consts.h"
#include "keymaster.h"
#include "message_block.h"
#include "formatter.h"
// #include "error.h"

#define NON_REALTIME_STATUS(b) ((b) >= NOTE_OFF && (b) <= EOX)

MessageBlock::MessageBlock(DBObjID id, const String &name)
  : DBObj(id), Nameable(name)
{
}

// Reads `str` which must consist of bytes encoded as two-digit hex strings
// in groups of four bytes and converts those to MidiMessages. Each group of
// four represents the bytes in little-endian order (high byte first).
// Replaces the current messages.
void MessageBlock::from_string(const String &str) {
  String copy(str);
  StringArray tokens;
  while (copy.isNotEmpty()) {
    tokens.add(copy.substring(0, 2));
    copy = copy.substring(2);
  }
  from_tokens(tokens);
}

// Reads `str` which must consist of bytes encoded as two-digit hex strings.
// Whitespace is skipped/ignored. Replaces the current messages.
void MessageBlock::from_editable_string(const String &str) {
  StringArray tokens;
  tokens.addTokens(str, false);
  from_tokens(tokens);
}

void MessageBlock::from_tokens(StringArray &tokens) {
  _midi_messages.clear();

  int i = 0;
  while (i < tokens.size()) {
    String word = tokens[i++];
    int status = (juce::uint8)word.getHexValue32();
    int switch_status = (status < 0xf0 && status >= 0x80) ? (status & 0xf0) : status;
    juce::uint8 data1, data2; // data3
    // bool in_sysex = false;

  RESWITCH:
    switch (switch_status) {
    case NOTE_OFF: case NOTE_ON: case POLY_PRESSURE: case CONTROLLER:
    case PITCH_BEND: case SONG_POINTER:
      word = tokens[i++]; data1 = (juce::uint8)word.getHexValue32();
      word = tokens[i++]; data2 = (juce::uint8)word.getHexValue32();
      _midi_messages.addEvent(MidiMessage(status, data1, data2), 0);
      break;
    case PROGRAM_CHANGE: case CHANNEL_PRESSURE: case SONG_SELECT:
      word = tokens[i++]; data1 = (juce::uint8)word.getHexValue32();
      _midi_messages.addEvent(MidiMessage(status, data1), 0);
      break;
    case TUNE_REQUEST:
      _midi_messages.addEvent(MidiMessage(status), 0);
      break;
    case SYSEX:
      // in_sysex = true;
      // while (in_sysex) {
      //   data1 = data2 = data3 = 0;
      //   if (NON_REALTIME_STATUS(status) && status != SYSEX) {
      //     in_sysex = false;
      //     if (data1 == EOX) goto SYSEX_EOX;
      //     goto RESWITCH;
      //   }

      //   word = tokens[i++]; data1 = hex_to_byte(word);
      //   if (NON_REALTIME_STATUS(data1)) {
      //     in_sysex = false;
      //     if (data1 == EOX) goto SYSEX_EOX;
      //     event.message = Pm_Message(status, 0, 0);
      //     __midi_messages.add(event);
      //     status = data1;
      //     goto RESWITCH;
      //   }

      //   word = tokens[i++]; data2 = hex_to_byte(word);
      //   if (NON_REALTIME_STATUS(data2)) {
      //     in_sysex = false;
      //     if (data1 == EOX) goto SYSEX_EOX;
      //     event.message = Pm_Message(status, data1, 0);
      //     __midi_messages.add(event);
      //     status = data2;
      //     goto RESWITCH;
      //   }

      //   word = tokens[i++]; data3 = hex_to_byte(word);
      //   if (NON_REALTIME_STATUS(data3)) {
      //     in_sysex = false;
      //     if (data1 == EOX) goto SYSEX_EOX;
      //     event.message = Pm_Message(status, data1, data2);
      //     __midi_messages.add(event);
      //     status = data3;
      //     goto RESWITCH;
      //   }

      // SYSEX_EOX:
      //   event.message = Pm_Message(status, data1, data2) + (data3 << 24);
      //   __midi_messages.add(event);

      //   if (in_sysex) {
      //     word = tokens[i++]; status = hex_to_byte(word);
      //   }
      // }
      break;
    default:
      // error_message(String::formatted("bad MIDI data seen in string; expected status byte got %02x", status));
      break;
    }
    word = tokens[i++];
  }
}

// Returns a string consisting of non-delimited two-digit hex bytes.
String MessageBlock::to_hex_string() {
  String str;
  for (auto &msg : _midi_messages)
    str += String::toHexString(msg->message.getRawData(), msg->message.getRawDataSize());
  return str;
}

// Returns a string consisting of space- and newline-delimited two-digit hex
// numbers. Each message is separated by a newline and each byte within the
// message is separated by a space.
String MessageBlock::to_editable_hex_string() {
  StringArray strs;

  for (auto &msg : _midi_messages) {
    StringArray msg_arr;
    for (int i = 0; i < msg->message.getRawDataSize(); ++i)
      msg_arr.add(String::toHexString(msg->message.getRawData() + i, 1));
    strs.add(msg_arr.joinIntoString(" "));
  }
  return strs.joinIntoString("\n");
}

void MessageBlock::send_to_all_outputs() {
  for (auto &out : KeyMaster_instance()->outputs())
    send_to(out);
}

void MessageBlock::send_to(MidiOutputEntry::Ptr out) {
  for (auto &msg : _midi_messages)
    out->device->sendMessageNow(msg->message);
}
