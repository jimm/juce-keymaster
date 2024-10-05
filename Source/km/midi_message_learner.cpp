#include "midi_message_learner.h"

#define is_realtime(b) ((b) >= 0xf8)

MidiMessageLearner::MidiMessageLearner(bool clock_ok, bool active_sense_ok, bool realtime_ok, bool sysex_ok)
  : _clock_ok(clock_ok), _active_sense_ok(active_sense_ok), _realtime_ok(realtime_ok), _sysex_ok(sysex_ok)
{
}

bool MidiMessageLearner::wants_message(const MidiMessage &message) {
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

