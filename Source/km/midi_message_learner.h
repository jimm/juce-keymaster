#pragma once

#include <JuceHeader.h>

class MidiMessageLearner {
public:
  MidiMessageLearner(bool clock_ok, bool active_sense_ok, bool realtime_ok, bool sysex_ok);
  virtual ~MidiMessageLearner() {}

  bool wants_message(const MidiMessage &message);

  virtual void learnMidiMessage(const MidiMessage &message) = 0;

private:
  bool _clock_ok;
  bool _active_sense_ok;
  bool _realtime_ok;
  bool _sysex_ok;
};
