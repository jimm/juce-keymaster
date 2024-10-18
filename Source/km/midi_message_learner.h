#pragma once

#include <JuceHeader.h>
#include "input.h"

class MidiMessageLearner {
public:
  MidiMessageLearner(bool clock_ok, bool active_sense_ok, bool realtime_ok, bool sysex_ok);
  virtual ~MidiMessageLearner();

  void start_learning(Input::Ptr inp);
  bool is_learning() { return _input != nullptr; }
  bool wants_midi_message(const MidiMessage &message);
  // Overrideable so you can call stop_learning when you want to
  virtual void learn_midi_message(const MidiMessage &message);
  void stop_learning();

  Array<MidiMessage> &midi_messages() { return _midi_messages; }

private:
  bool _clock_ok;
  bool _active_sense_ok;
  bool _realtime_ok;
  bool _sysex_ok;
  Input::Ptr _input;
  Array<MidiMessage> _midi_messages;
};
