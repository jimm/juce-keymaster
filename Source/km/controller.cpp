#include <math.h>
#include "consts.h"
#include "controller.h"

const MidiMessage EMPTY_MESSAGE = MidiMessage(ACTIVE_SENSE);

Controller::Controller(DBObjID id, int num)
  : DBObj(id),
    _cc_num(num),
    _translated_cc_num(num),
    _filtered(false),
    _pass_through_0(true),
    _pass_through_127(true),
    _min_in(0),
    _max_in(127),
    _min_out(0),
    _max_out(127)
{
}

void Controller::set_range(bool pass_0, bool pass_127,
                           int min_in, int max_in,
                           int min_out, int max_out)
{
  _pass_through_0 = pass_0;
  _pass_through_127 = pass_127;
  _min_in = min_in;
  _min_out = min_out;
  _max_in = max_in;
  _max_out = max_out;

  float new_slope = ((float)_max_out - (float)_min_out) / ((float)_max_in - (float)_min_in);
  _slope = new_slope;
}

MidiMessage Controller::process(const MidiMessage &msg, int output_chan) {
  if (_filtered)
    return EMPTY_MESSAGE;

  int chan = (output_chan != CONNECTION_ALL_CHANNELS)
    ? output_chan
    : msg.getChannel();
  int data2 = msg.getRawData()[2];

  int new_val = process_data_byte(data2);
  if (new_val == CONTROLLER_BLOCK)
    return EMPTY_MESSAGE;
  return MidiMessage::controllerEvent(chan, _translated_cc_num, new_val);
}

int Controller::process_data_byte(int val) {
  // simplest case: no change in value
  if (_min_in == 0 && _max_in == 127 &&
      _min_out == 0 && _max_out == 127)
    return val;

  // pass-through 0 or 127 values if requested
  if (val == 0 && _pass_through_0)
    return 0;
  if (val == 127 && _pass_through_127)
    return 127;

  // input value out of range, filter out
  if (val < _min_in || val > _max_in)
    return CONTROLLER_BLOCK;

  return _min_out + floor((_slope * (val - _min_in)) + 0.5);
}
