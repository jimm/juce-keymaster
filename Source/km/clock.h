#pragma once

#include <JuceHeader.h>

class Clock : public ActionBroadcaster, HighResolutionTimer {
public:
  Clock();
  virtual ~Clock() override;

  float bpm() const { return _bpm; }
  void set_bpm(float bpm);

  void start();
  void stop();
  bool is_running() { return isTimerRunning(); }

  void hiResTimerCallback() override;

protected:
  float _bpm;
  long _millisecs_per_tick;
  bool _running;
  int _tick;
};
