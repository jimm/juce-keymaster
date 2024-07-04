#pragma once

#include <JuceHeader.h>
#include "connection.h"

String note_num_to_name(int num);
int note_name_to_num(String &str); // str may point to an integer string like "64" as well

// Translates up to first two hex chars into an unsigned char value. Zero,
// one, or two chars used.
juce::int8 hex_to_byte(String &hex);

// Reads two-digit hex chars and converts them to bytes, returning a newly
// allocated buffer. Ignores any non-hex characters in `hex`.
Array<juce::uint8> hex_to_bytes(String &hex);

// Converts `bytes` into two-digit hex characters and returns a String.
String bytes_to_hex(unsigned char *bytes, int len);

// Translates hex bytes in `str` into a single non-sysex MIDI message. If
// `str` is `nullptr` returns a message consisting of three zero bytes.
juce::MidiMessage message_from_string(String &str);
