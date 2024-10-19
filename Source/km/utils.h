#pragma once

#include <JuceHeader.h>

#define is_status(b) (((b) & 0x80) == 0x80)
#define is_channel(b) ((b) >= 0x80 && (b) < 0xf0)
#define is_system(b) ((b) >= SYSEX)
#define is_realtime(b) ((b) >= 0xf8)

bool mm_equal(const MidiMessage &a, const MidiMessage &b);
