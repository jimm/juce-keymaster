#include <JuceHeader.h>
#include "clock.h"
#include "keymaster.h"

#define BPM_TO_CLOCK_TICK_MS(bpm) long(2500.0 / float(bpm))

static MidiMessage CLOCK_MESSAGE = MidiMessage::midiClock();

Clock::Clock() {
  set_bpm(120);
}

Clock::~Clock() {
  if (is_running())
    stop();
}

void Clock::set_bpm(float new_val) {
  if (std::abs(_bpm - new_val) > 0.001) {
    _bpm = new_val;
    _millisecs_per_tick = BPM_TO_CLOCK_TICK_MS(_bpm);
    sendActionMessage("clock:bpm");
  }
}

void Clock::start() {
  if (is_running())
    return;
  _tick = 23;                   // next tick will be on the beat
  startTimer((int)_millisecs_per_tick);
  sendActionMessage("clock:start");
}

void Clock::stop() {
  if (!is_running())
    return;
  stopTimer();
  sendActionMessage("clock:stop");
}

// Sends CLOCK message downstream and returns the amount of time to wait
// until the next tick, in _milliseconds.
void Clock::hiResTimerCallback() {
  for (auto &output : KeyMaster_instance()->device_manager().outputs())
    output->midi_out(CLOCK_MESSAGE);
  if (_tick == 0)
    sendActionMessage("clock:beat");
  else if (_tick == CLOCK_TICKS_PER_QUARTER_NOTE / 4)
    sendActionMessage("clock:unbeat");
  if (++_tick == CLOCK_TICKS_PER_QUARTER_NOTE) {
    _tick = 0;
  }
}
