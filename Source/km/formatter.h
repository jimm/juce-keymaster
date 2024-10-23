#pragma once

#include <JuceHeader.h>

int note_name_to_num(String str); // str may point to an integer string like "64" as well

String trigger_message_description(const MidiMessage &msg);

String to_hex(const Array<MidiMessage> &messages, String message_separator);
