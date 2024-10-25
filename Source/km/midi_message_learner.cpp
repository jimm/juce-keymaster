#include "keymaster.h"
#include "midi_message_learner.h"

MidiMessageLearner::MidiMessageLearner(bool sysex_ok, bool pitch_bend_ok, bool clock_ok, bool active_sense_ok)
  : _sysex_ok(sysex_ok), _pitch_bend_ok(pitch_bend_ok),
    _clock_ok(clock_ok), _active_sense_ok(active_sense_ok),
    _learning(false), _max_messages(0), _max_callback([]{})
{
}

MidiMessageLearner::~MidiMessageLearner() {
  stop_learning();
}

void MidiMessageLearner:: start_learning(int max_messages, std::function<void()> max_callback) {
  _midi_messages.clear();
  _max_messages = max_messages;
  _max_callback = max_callback;
  _learning = true;
  _num_messages = 0;
  if (KeyMaster_instance() != nullptr) // null during testing
    for (auto inp : KeyMaster_instance()->device_manager().inputs())
      inp->add_listener(this);
}

bool MidiMessageLearner::want_midi_message(const MidiMessage &message) {
  if (message.isPitchWheel())
    return _pitch_bend_ok;
  if (message.isMidiClock())
    return _clock_ok;
  if (message.isActiveSense())
    return _active_sense_ok;
  if (message.isSysEx())
    return _sysex_ok;
  return true;
}

void MidiMessageLearner::learn_midi_message(const MidiMessage &message) {
  if (!_learning || !want_midi_message(message))
    return;

  _midi_messages.add(message);
  if (_max_messages > 0 && ++_num_messages >= _max_messages) {
    stop_learning();
    _max_callback();
  }
}

void MidiMessageLearner::stop_learning() {
  _learning = false;
  if (KeyMaster_instance() != nullptr) // null during testing
    for (auto inp : KeyMaster_instance()->device_manager().inputs())
      inp->remove_listener(this);
}

void MidiMessageLearner::reset(bool sysex_ok, bool pitch_bend_ok, bool clock_ok, bool active_sense_ok) {
  _sysex_ok = sysex_ok;
  _pitch_bend_ok = pitch_bend_ok;
  _clock_ok = clock_ok;
  _active_sense_ok = active_sense_ok;
  _learning = false;
  _max_messages = 0;
  _max_callback = []{};
  _learning = false;
  _midi_messages.clear();
  _num_messages = 0;
}
