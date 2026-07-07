#pragma once

#include <JuceHeader.h>
#include "consts.h"
#include "db_obj.h"
#include "input.h"
#include "output.h"
#include "message_filter.h"
#include "controller.h"
#include "curve.h"

struct ProgramChange {
  Output::Ptr output;
  int chan;       // 0-15 (internally), or CONNECTION_ALL_CHANNELS
  int bank_msb;  // UNDEFINED or 0-127
  int bank_lsb;  // UNDEFINED or 0-127
  int prog;      // UNDEFINED or 0-127

  ProgramChange()
    : chan(CONNECTION_ALL_CHANNELS),
      bank_msb(UNDEFINED), bank_lsb(UNDEFINED), prog(UNDEFINED) {}
  ProgramChange(Output::Ptr out, int ch, int msb, int lsb, int p)
    : output(out), chan(ch), bank_msb(msb), bank_lsb(lsb), prog(p) {}
};

typedef struct zone {
  int low;
  int high;
} zone;

class Connection : public DBObj {
public:
  Connection(DBObjID id, Input::Ptr input, int in_chan, Output::Ptr output, int out_chan);
  Connection(const Connection &other) noexcept;
  ~Connection();

  inline Input::Ptr input() const { return _input; }
  inline Output::Ptr output() const { return _output; }
  inline int input_chan() const { return _input_chan; }
  inline int output_chan() const { return _output_chan; }
  inline int zone_low() const { return _zone.low; }
  inline int zone_high() const { return _zone.high; }
  inline int xpose() const { return _xpose; }
  inline Curve *velocity_curve() const { return _velocity_curve; }
  inline MessageFilter &message_filter() { return _message_filter; }
  inline bool running() const { return _running; }
  inline bool changing_was_running() const { return _changing_was_running; }
  inline Controller *cc_map(int i) const { return _cc_maps[i]; }

  void set_input(Input::Ptr val);
  void set_output(Output::Ptr val);
  void set_input_chan(int val);
  void set_output_chan(int val);
  void set_zone_low(int val);
  void set_zone_high(int val);
  void set_xpose(int val);
  void set_velocity_curve(Curve *val);
  void set_running(bool val);
  void set_cc_map(int cc_num, Controller *val);

  const OwnedArray<ProgramChange> &program_changes() const { return _program_changes; }
  OwnedArray<ProgramChange> &program_changes() { return _program_changes; }
  void add_program_change(ProgramChange *pc);
  void remove_program_change(int index);
  void program_changes_changed();

  void start();
  bool is_running();
  void stop();

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

#ifdef JUCE_UNIT_TESTS
  virtual
#endif
  void start_program_changes();

private:
  Input::Ptr _input;
  Output::Ptr _output;
  int _input_chan;
  int _output_chan;
  OwnedArray<ProgramChange> _program_changes;
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
