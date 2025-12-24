#pragma once

#include <JuceHeader.h>

class MidiListener {
public:
  virtual ~MidiListener() {}

  virtual void midi_input(const String &, const MidiMessage &) {}
  virtual void midi_output(const String &, const MidiMessage &) {}
};
