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
  midi_messages.clear();

  int i = 0;
  while (i < tokens.size()) {
    String word = tokens[i++];
    int status = hex_to_byte(word);
    int switch_status = (status < 0xf0 && status >= 0x80) ? (status & 0xf0) : status;
    juce::uint8 data1, data2; // data3
    // bool in_sysex = false;

  RESWITCH:
    switch (switch_status) {
    case NOTE_OFF: case NOTE_ON: case POLY_PRESSURE: case CONTROLLER:
    case PITCH_BEND: case SONG_POINTER:
      word = tokens[i++]; data1 = hex_to_byte(word);
      word = tokens[i++]; data2 = hex_to_byte(word);
      midi_messages.addEvent(MidiMessage(status, data1, data2), 0);
      break;
    case PROGRAM_CHANGE: case CHANNEL_PRESSURE: case SONG_SELECT:
      word = tokens[i++]; data1 = hex_to_byte(word);
      midi_messages.addEvent(MidiMessage(status, data1), 0);
      break;
    case TUNE_REQUEST:
      midi_messages.addEvent(MidiMessage(status), 0);
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
      //     _midi_messages.add(event);
      //     status = data1;
      //     goto RESWITCH;
      //   }

      //   word = tokens[i++]; data2 = hex_to_byte(word);
      //   if (NON_REALTIME_STATUS(data2)) {
      //     in_sysex = false;
      //     if (data1 == EOX) goto SYSEX_EOX;
      //     event.message = Pm_Message(status, data1, 0);
      //     _midi_messages.add(event);
      //     status = data2;
      //     goto RESWITCH;
      //   }

      //   word = tokens[i++]; data3 = hex_to_byte(word);
      //   if (NON_REALTIME_STATUS(data3)) {
      //     in_sysex = false;
      //     if (data1 == EOX) goto SYSEX_EOX;
      //     event.message = Pm_Message(status, data1, data2);
      //     _midi_messages.add(event);
      //     status = data3;
      //     goto RESWITCH;
      //   }

      // SYSEX_EOX:
      //   event.message = Pm_Message(status, data1, data2) + (data3 << 24);
      //   _midi_messages.add(event);

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

// Returns a string consisting of non-delimited two-digit hex bytes. Every
// PmMessage is converted to four bytes, no matter what the length of the
// encoded MIDI message. The bytes are little-endian, so for every group of
// four bytes the first byte in the string is the highest byte in the
// PmMessage (which is really an int32).
String MessageBlock::to_string() {
  String string;
  // char buf[9];

  // buf[8] = 0;
  // for (auto &event : midi_messages) {
  //   snprintf(buf, 9, "%08x", event.message);
  //   string += buf;
  // }
  return string;
}

// Returns a string consisting of space- and newline-delimited two-digit hex
// numbers. Each message is separated by a newline and each byte within the
// message is separated by a space.
String MessageBlock::to_editable_string() {
  // char buf[BUFSIZ];
  String str;
  // bool in_sysex = false;

  // TODO use StringArray then call .joinIntoString(" ");

  // for (auto &event : midi_messages) {
  //   PmMessage msg = event.message;
  //   int status = Pm_MessageStatus(msg);
  //   int switch_status = (status < 0xf0 && status >= 0x80) ? (status & 0xf0) : status;
  //   int byte;

  //   switch (switch_status) {
  //   case NOTE_OFF: case NOTE_ON: case POLY_PRESSURE: case CONTROLLER:
  //   case PITCH_BEND: case SONG_POINTER:
  //     if (in_sysex) {
  //       in_sysex = false;
  //       str += "f7\n";
  //     }
  //     snprintf(buf, 10, "%02x %02x %02x\n", (int)status,
  //             (int)Pm_MessageData1(msg),
  //             (int)Pm_MessageData2(msg));
  //     str += buf;
  //     break;
  //   case PROGRAM_CHANGE: case CHANNEL_PRESSURE: case SONG_SELECT:
  //     if (in_sysex) {
  //       in_sysex = false;
  //       str += "f7\n";
  //     }
  //     snprintf(buf, 8, "%02x %02x\n", (int)status,
  //             (int)Pm_MessageData1(msg));
  //     str += buf;
  //     break;
  //   case TUNE_REQUEST:
  //     if (in_sysex) {
  //       in_sysex = false;
  //       str += "f7\n";
  //     }
  //     str += "f6\n";
  //     break;
  //   case SYSEX:
  //     if (in_sysex)
  //       str += " f7\n";         // EOX from previous SYSEX
  //     in_sysex = true;
  //   PRINT_SYSEX_FOUR_BYTES:
  //     if (status == EOX) goto EOX_SEEN;
  //     snprintf(buf, 3, "%02x", status);
  //     str += buf;

  //     byte = Pm_MessageData1(msg);
  //     if (byte == EOX) goto EOX_SEEN;
  //     snprintf(buf, 4, " %02x", byte);
  //     str += buf;

  //     byte = Pm_MessageData2(msg);
  //     if (byte == EOX) goto EOX_SEEN;
  //     snprintf(buf, 4, " %02x", byte);
  //     str += buf;

  //     byte = (msg >> 24) & 0xff;
  //     if (byte == EOX) goto EOX_SEEN;
  //     snprintf(buf, 4, " %02x", byte);
  //     str += buf;

  //     break;

  //   EOX_SEEN:
  //     in_sysex = false;
  //     str += " f7\n";
  //     break;
  //   default:
  //     if (in_sysex) {
  //       str += ' ';
  //       goto PRINT_SYSEX_FOUR_BYTES;
  //     }
  //     snprintf(buf, 4, "%02x", status);
  //     str += buf;
  //     break;
  //   }
  // }
  return str;
}

void MessageBlock::send_to_all_outputs() {
  for (auto &out : KeyMaster_instance()->outputs())
    send_to(out);
}

void MessageBlock::send_to(MidiOutputEntry::Ptr out) {
  out->device->sendBlockOfMessagesNow(midi_messages);
}
