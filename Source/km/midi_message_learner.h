#pragma once

#include <JuceHeader.h>

class MidiMessageLearner : public MidiListener {
public:
  MidiMessageLearner(
    bool sysex_ok, bool pitch_bend_ok = false, bool clock_ok = false, bool active_sense_ok = false
  );
  virtual ~MidiMessageLearner() override;

  // If max_messages = 0, keeps learning until stop_learning is called.
  // Else, stops after that many messages and calls max_callback when the
  // limit is reached.
  void start_learning(int max_messages = 0, std::function<void()> max_callback = []{}); // 0 means no limit

  bool is_learning() { return _learning; }

  virtual void midi_input(const String &, const MidiMessage &message) override { learn_midi_message(message); }

  // Called by learn_midi_message. You don't have to call this; it's only
  // public so it's easier to test.
  bool want_midi_message(const MidiMessage &message);

  // Overridable so you can do what you want when a message arrives.
  // Recommend you call this first; the message may be rejected so don't
  // assume it's been added to _midi_messages.
  virtual void learn_midi_message(const MidiMessage &message);

  void stop_learning();

  Array<MidiMessage> &midi_messages() { return _midi_messages; }

  // For testing
  void reset(
    bool sysex_ok = false, bool pitch_bend_ok = false, bool clock_ok = false, bool active_sense_ok = false
  );

private:
  bool _sysex_ok;
  bool _pitch_bend_ok;
  bool _clock_ok;
  bool _active_sense_ok;
  bool _learning;
  int _max_messages;
  int _num_messages;
  std::function<void()> _max_callback;
  Array<MidiMessage> _midi_messages;
};
