#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <JuceHeader.h>
#include "formatter.h"
#include "consts.h"
#include "connection.h"
#include "controller.h"
static const int NOTE_OFFSETS[] = {
  9, 11, 0, 2, 4, 5, 7
};

static const char HEX_DIGITS[] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

String note_num_to_name(int num) {
  return MidiMessage::getMidiNoteName(num, true, true, 4);
}

// str may point to an integer string like "64" as well
int note_name_to_num(String &str) {
  char ch = (char)str[0];

  if (isdigit(ch))
    return str.getIntValue();

  ch = tolower(ch);
  if (ch < 'a' || ch > 'g')
    return 0;

  int from_c = NOTE_OFFSETS[ch - 'a'];
  int num_start = 1;
  int accidental = 0;
  switch (tolower((char)str[1])) {
  case 's': case '#':
    accidental = 1;
    ++num_start;
    break;
  case 'f': case 'b':
    accidental = -1;
    ++num_start;
    break;
  }

  int octave = (str.substring(num_start).getIntValue() + 1) * 12;
  return octave + from_c + accidental;
}

String format_program(int bank_msb, int bank_lsb, int prog) {
  String str;
  int has_msb = bank_msb != UNDEFINED;
  int has_lsb = bank_lsb != UNDEFINED;
  int has_bank = has_msb || has_lsb;

  str += String::formatted(" %c", has_bank ? '[' : ' ');

  if (has_msb)
    str += String::formatted("%3d", bank_msb);
  else
    str += "   ";

  str += String::formatted("%c ", has_bank ? ',' : ' ');

  if (has_lsb)
    str += String::formatted("%3d", bank_lsb);
  else
    str += "   ";

  str += String::formatted("%c ", has_bank ? ']' : ' ');

  if (prog != UNDEFINED)
    str += String::formatted(" %3d", prog);
  else
    str += "    ";

  return str;
}

// ================ parsing MIDI messages ================

// Translates up to first two hex chars into an int8 value. Zero,
// one, or two chars used.
int8 hex_to_byte(String &hex) {
  return (int8)hex.substring(0, 2).getHexValue32();
}

// private helper
//
bool check_byte_value(int val) {
  if (val >= 0 && val <= 255)
    return true;

  // error_message("byte value %d is out of range", val);
  return false;
}

// Reads two-digit hex chars and converts them to bytes, returning a newly
// allocated buffer.
Array<uint8> hex_to_bytes(String &hex) {
  Array<uint8> buf;

  while (hex.isNotEmpty()) {
    buf.add(hex_to_byte(hex));
    hex = hex.substring(2);
  }
  return buf;
}

// Converts `bytes` into two-digit hex characters and returns a newly
// allocated zero-terminated buffer.
String bytes_to_hex(int8 *bytes, int len) {
  String hex;

  int i = 0;
  while (i < len) {
    hex += HEX_DIGITS[(bytes[i++] >> 4) & 0x0f];
    hex += HEX_DIGITS[bytes[i++] & 0x0f];
  }
  return hex;
}

// Translates hex bytes in `str` into a single non-sysex MIDI message. If
// `str` is empty returns an active sensing message.
MidiMessage message_from_hex(String &hex) {
  if (hex.isEmpty())
    return MidiMessage(ACTIVE_SENSE);

  Array<uint8> data = hex_to_bytes(hex);
  return MidiMessage(data.data(), data.size());
}
