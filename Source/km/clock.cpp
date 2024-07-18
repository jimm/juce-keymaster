#include <JuceHeader.h>
#include "clock.h"
#include "keymaster.h"

static MidiMessage CLOCK_MESSAGE = MidiMessage::midiClock();

Clock::Clock() {
  set_bpm(120);
}

Clock::~Clock() {
  if (is_running())
    stop();
}

void Clock::set_bpm(float new_val) {
  if (_bpm != new_val) {
    _bpm = new_val;
    _millisecs_per_tick = (long)(2.5e6 / _bpm);
  }
}

void Clock::start() {
  if (is_running())
    return;
  startTimer((int)_millisecs_per_tick);
}

void Clock::stop() {
  if (!is_running())
    return;
  stopTimer();
}

// Sends CLOCK message downstream and returns the amount of time to wait
// until the next tick, in _milliseconds.
void Clock::hiResTimerCallback() {
  for (auto &output : KeyMaster_instance()->device_manager().outputs())
    output->midi_out(CLOCK_MESSAGE);
}
