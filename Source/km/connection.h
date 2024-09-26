#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "input.h"
#include "output.h"
#include "message_filter.h"
#include "controller.h"
#include "curve.h"

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
  Connection(DBObjID id, Input::Ptr input, int in_chan, Output::Ptr output, int out_chan);
  Connection(const Connection &other) noexcept;
  ~Connection();

  inline Input::Ptr input() { return _input; }
  inline Output::Ptr output() { return _output; }
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
  inline bool running() { return _running; }
  inline bool changing_was_running() { return _changing_was_running; }
  inline Controller *cc_map(int i) { return _cc_maps[i]; }

  void set_input(Input::Ptr val);
  void set_output(Output::Ptr val);
  void set_input_chan(int val);
  void set_output_chan(int val);
  void set_program_bank_msb(int val);
  void set_program_bank_lsb(int val);
  void set_program_prog(int val);
  void set_zone_low(int val);
  void set_zone_high(int val);
  void set_xpose(int val);
  void set_velocity_curve(Curve *val);
  void set_running(bool val);
  void set_cc_map(int cc_num, Controller *val);

  void start();
  bool is_running();
  void stop();

  // Returns CONNECTION_ALL_CHANNELS if we can't determine what channel to
  // send to (because both input and output don't declare channels). This
  // means that no program change will be sent.
  int program_change_send_channel();
  bool program_change_can_be_sent() {
    return program_change_send_channel() != CONNECTION_ALL_CHANNELS;
  }

  void begin_changes();
  void end_changes();

  void midi_in(Input::Ptr input, const MidiMessage& message);

  void set_controller(Controller *controller);
  void remove_cc_num(int cc_num);

protected:
#ifdef JUCE_UNIT_TESTS
  virtual
#endif
  void midi_out(MidiMessage *);

#ifdef JUCE_UNIT_TESTS
  virtual
#endif
  void midi_out(MidiMessage);

private:
  Input::Ptr _input;
  Output::Ptr _output;
  int _input_chan;
  int _output_chan;
  struct program _prog;
  struct zone _zone;
  int _xpose;
  Curve *_velocity_curve;
  MessageFilter _message_filter;
  bool _running;
  bool _changing_was_running;
  Controller *_cc_maps[128];

  int input_channel_ok(const MidiMessage &message);
  int inside_zone(int note);
};
