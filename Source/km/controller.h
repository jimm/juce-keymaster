#pragma once

#include <JuceHeader.h>
#include "db_obj.h"

class Controller : public DBObj {
public:
  Controller(DBObjID id, int cc_num);

  inline int cc_num() const { return _cc_num; }
  inline int translated_cc_num() const { return _translated_cc_num; }
  inline bool filtered() const { return _filtered; }
  inline bool pass_through_0() const { return _pass_through_0; }
  inline bool pass_through_127() const { return _pass_through_127; }
  inline int min_in() const { return _min_in; }
  inline int max_in() const { return _max_in; }
  inline int min_out() const { return _min_out; }
  inline int max_out() const { return _max_out; }

  void set_cc_num(int val);
  void set_translated_cc_num(int val);
  void set_filtered(bool val);
  void set_pass_through_0(bool val);
  void set_pass_through_127(bool val);

  void set_range(bool pass_0, bool pass_127,
                 int min_in, int max_in,
                 int min_out, int max_out);

  // Returns EMPTY_MESSAGE if nothing should be sent.
  MidiMessage process(const MidiMessage &msg, int output_channel);

private:
  int _cc_num;
  int _translated_cc_num;
  bool _filtered;
  bool _pass_through_0;
  bool _pass_through_127;
  int _min_in;
  int _max_in;
  int _min_out;
  int _max_out;
  float _slope;

  int process_data_byte(int val);
};
