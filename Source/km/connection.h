#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "message_filter.h"
#include "controller.h"
#include "curve.h"
#include "midi_device.h"

typedef struct program {
  int bank_msb;
  int bank_lsb;
  int prog;
} program;

typedef struct zone {
  int low;
  int high;
} zone;
  
class Connection : public DBObj {
public:
  Connection(DBObjID id, MidiInputEntry::Ptr in, int in_chan, MidiOutputEntry::Ptr out, int out_chan);
  ~Connection();

  inline MidiInputEntry::Ptr input() { return _input; }
  inline MidiOutputEntry::Ptr output() { return _output; }
  inline int input_chan() { return _input_chan; }
  inline int output_chan() { return _output_chan; }
  inline int program_bank_msb() { return _prog.bank_msb; }
  inline int program_bank_lsb() { return _prog.bank_lsb; }
  inline int program_prog() { return _prog.prog; }
  inline int zone_low() { return _zone.low; }
  inline int zone_high() { return _zone.high; }
  inline int xpose() { return _xpose; }
  inline Curve *velocity_curve() { return _velocity_curve; }
  inline MessageFilter &message_filter() { return _message_filter; }
  inline bool processing_sysex() { return _processing_sysex; }
  inline bool running() { return _running; }
  inline bool changing_was_running() { return _changing_was_running; }
  inline Controller *cc_map(int i) { return _cc_maps[i]; }

  inline void set_input(MidiInputEntry::Ptr val) { _input = val; }
  inline void set_output(MidiOutputEntry::Ptr val) { _output = val; }
  inline void set_input_chan(int val) { _input_chan = val; }
  inline void set_output_chan(int val) { _output_chan = val; }
  inline void set_program_bank_msb(int val) { _prog.bank_msb = val; }
  inline void set_program_bank_lsb(int val) { _prog.bank_lsb = val; }
  inline void set_program_prog(int val) { _prog.prog = val; }
  inline void set_zone_low(int val) { _zone.low = val; }
  inline void set_zone_high(int val) { _zone.high = val; }
  inline void set_xpose(int val) { _xpose = val; }
  inline void set_velocity_curve(Curve *val) { _velocity_curve = val; }
  inline void set_processing_sysex(bool val) { _processing_sysex = val; }
  inline void set_running(bool val) { _running = val; }
  inline void set_cc_map(int cc_num, Controller *val) { _cc_maps[cc_num] = val; }

  void start();
  bool is_running();
  void stop();

  // Returns CONNECTION_ALL_CHANNELS if we can't determine what channel to
  // send to (because both input and output don't declare channels). This
  // means that no program change will be sent.
  int program_change_send_channel();

  void begin_changes();
  void end_changes();

  void midi_in(MidiInput* source, const MidiMessage& message);

  void set_controller(Controller *controller);
  void remove_cc_num(int cc_num);

private:
  MidiInputEntry::Ptr _input;
  MidiOutputEntry::Ptr _output;
  int _input_chan;
  int _output_chan;
  struct program _prog;
  struct zone _zone;
  int _xpose;
  Curve *_velocity_curve;
  MessageFilter _message_filter;
  bool _processing_sysex;
  bool _running;
  bool _changing_was_running;
  Controller *_cc_maps[128];

  int input_channel_ok(int status);
  int inside_zone(int note);

  void midi_out(MidiMessage *);
  void midi_out(MidiMessage);
};
