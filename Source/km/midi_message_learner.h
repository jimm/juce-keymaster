#pragma once

#include <JuceHeader.h>
#include "input.h"

class MidiMessageLearner {
public:
  MidiMessageLearner(bool clock_ok, bool active_sense_ok, bool other_realtime_ok, bool sysex_ok);
  virtual ~MidiMessageLearner();

  void learn_one_message();

  void start_learning(int max_messages = 0, std::function<void()> max_callback = []{}); // 0 means no limit
  bool is_learning() { return _learning; }
  // Overrideable so you can call stop_learning when you want to
  virtual void learn_midi_message(const MidiMessage &message);
  void stop_learning();

  Array<MidiMessage> &midi_messages() { return _midi_messages; }

private:
  bool _clock_ok;
  bool _active_sense_ok;
  bool _other_realtime_ok;
  bool _sysex_ok;
  bool _learning;
  int _max_messages;
  int _num_messages;
  std::function<void()> _max_callback;
  Array<MidiMessage> _midi_messages;

  bool want_midi_message(const MidiMessage &message);
};
