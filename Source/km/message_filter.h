#pragma once

class MessageFilter {
public:
  // true means allow, false means filter out

  MessageFilter();

  bool note() { return _note; } // both on and off
  bool poly_pressure() { return _poly_pressure; }
  bool chan_pressure() { return _chan_pressure; }
  bool program_change() { return _program_change; }
  bool pitch_bend() { return _pitch_bend; }
  bool controller() { return _controller; }
  bool song_pointer() { return _song_pointer; }
  bool song_select() { return _song_select; }
  bool tune_request() { return _tune_request; }
  bool sysex() { return _sysex; }
  bool clock() { return _clock; }
  bool start_continue_stop() { return _start_continue_stop; }
  bool system_reset() { return _system_reset; }

  inline void set_note(bool val) { _note = val; } // both on and off
  inline void set_poly_pressure(bool val) { _poly_pressure = val; }
  inline void set_chan_pressure(bool val) { _chan_pressure = val; }
  inline void set_program_change(bool val) { _program_change = val; }
  inline void set_pitch_bend(bool val) { _pitch_bend = val; }
  inline void set_controller(bool val) { _controller = val; }
  inline void set_song_pointer(bool val) { _song_pointer = val; }
  inline void set_song_select(bool val) { _song_select = val; }
  inline void set_tune_request(bool val) { _tune_request = val; }
  inline void set_sysex(bool val) { _sysex = val; }
  inline void set_clock(bool val) { _clock = val; }
  inline void set_start_continue_stop(bool val) { _start_continue_stop = val; }
  inline void set_system_reset(bool val) { _system_reset = val; }

  bool filter_out(int status, int data1);

private:
  bool _note;                   // both on and off
  bool _poly_pressure;
  bool _chan_pressure;
  bool _program_change;
  bool _pitch_bend;
  bool _controller;
  bool _song_pointer;
  bool _song_select;
  bool _tune_request;
  bool _sysex;
  bool _clock;
  bool _start_continue_stop;
  bool _system_reset;
};