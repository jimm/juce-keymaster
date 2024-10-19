#include "midi_message_learner.h"
#include "keymaster.h"
#include "utils.h"

MidiMessageLearner::MidiMessageLearner(
  bool clock_ok, bool active_sense_ok, bool other_realtime_ok, bool sysex_ok
)
  : _clock_ok(clock_ok), _active_sense_ok(active_sense_ok),
    _other_realtime_ok(other_realtime_ok), _sysex_ok(sysex_ok),
    _learning(false), _max_messages(0)
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
  for (auto inp : KeyMaster_instance()->device_manager().inputs())
    inp->start_learning(this);
}

bool MidiMessageLearner::want_midi_message(const MidiMessage &message) {
  if (message.isMidiClock())
    return _clock_ok;
  if (message.isActiveSense())
    return _active_sense_ok;
  if (is_realtime(message.getRawData()[0]))
    return _other_realtime_ok;
  if (message.isSysEx())
    return _sysex_ok;
  return true;
}

void MidiMessageLearner::learn_midi_message(const MidiMessage &message) {
  if (want_midi_message(message))
    _midi_messages.add(message);
  if (_max_messages > 0 && ++_num_messages >= _max_messages) {
    stop_learning();
    _max_callback();
  }
}

void MidiMessageLearner::stop_learning() {
  _learning = false;
  for (auto inp : KeyMaster_instance()->device_manager().inputs())
    inp->stop_learning();
}
