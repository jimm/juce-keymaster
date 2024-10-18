#include "midi_message_learner.h"

#define is_realtime(b) ((b) >= 0xf8)

MidiMessageLearner::MidiMessageLearner(bool clock_ok, bool active_sense_ok, bool realtime_ok, bool sysex_ok)
  : _clock_ok(clock_ok), _active_sense_ok(active_sense_ok), _realtime_ok(realtime_ok), _sysex_ok(sysex_ok),
    _input(nullptr)
{
}

MidiMessageLearner::~MidiMessageLearner() {
  stop_learning();
}

void MidiMessageLearner:: start_learning(Input::Ptr inp) {
  _midi_messages.clear();
  _input = inp;
  if (_input != nullptr)
    _input->start_learning(this);
}

bool MidiMessageLearner::wants_midi_message(const MidiMessage &message) {
  if (message.isMidiClock())
    return _clock_ok;
  if (message.isActiveSense())
    return _active_sense_ok;
  if (is_realtime(message.getRawData()[0]))
    return _realtime_ok;
  if (message.isSysEx())
    return _sysex_ok;
  return true;
}

void MidiMessageLearner::learn_midi_message(const MidiMessage &message) {
  _midi_messages.add(message);
}

void MidiMessageLearner::stop_learning() {
  if (_input != nullptr) {
    _input->stop_learning();
    _input = nullptr;
  }
}
