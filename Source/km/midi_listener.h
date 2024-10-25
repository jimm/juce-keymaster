#pragma once

#include <JuceHeader.h>

class MidiListener {
public:
  virtual ~MidiListener() {}

  virtual void midi_input(const String &name, const MidiMessage &message) {}
  virtual void midi_output(const String &name, const MidiMessage &message) {}
};
