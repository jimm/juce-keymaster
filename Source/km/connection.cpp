#include <stdlib.h>
#include "consts.h"
#include "connection.h"
#include "keymaster.h"
#include "utils.h"

Connection::Connection(DBObjID id, Input::Ptr input, int in_chan, Output::Ptr output, int out_chan)
  : DBObj(id),
    _input(input), _output(output),
    _input_chan(in_chan), _output_chan(out_chan),
    _xpose(0), _velocity_curve(nullptr),
    _running(false), _changing_was_running(false)
{
  _prog.bank_msb = _prog.bank_lsb = _prog.prog = UNDEFINED;
  _zone = { 0, 127 };
  for (int i = 0; i < 128; ++i)
    _cc_maps[i] = nullptr;
}

Connection::Connection(const Connection &other) noexcept
  : DBObj(0),                   // don't need to worry about this; not saving/loading
    _input(other._input), _output(other._output),
    _input_chan(other._input_chan), _output_chan(other._output_chan),
    _prog(other._prog),
    _zone(other._zone),
    _xpose(other._xpose), _velocity_curve(other._velocity_curve),
    _running(false), _changing_was_running(false)
{
    for (int i = 0; i < 128; ++i)
    _cc_maps[i] = other._cc_maps[i];
}

Connection::~Connection() {
  _input = nullptr;
  _output = nullptr;
  for (int i = 0; i < 128; ++i)
    if (_cc_maps[i] != nullptr)
      delete _cc_maps[i];
}

void Connection::start() {
  if (_running)
    return;

  _running = true;

  int chan = program_change_send_channel();
  if (chan != CONNECTION_ALL_CHANNELS) {
    if (_prog.bank_msb >= 0)
      midi_out(MidiMessage::controllerEvent(JCH(chan), CC_BANK_SELECT_MSB, _prog.bank_msb));
    if (_prog.bank_lsb >= 0)
      midi_out(MidiMessage::controllerEvent(JCH(chan), CC_BANK_SELECT_LSB, _prog.bank_lsb));
    if (_prog.prog >= 0)
      midi_out(MidiMessage::programChange(JCH(chan), _prog.prog));
  }
}

bool Connection::is_running() {
  return _running;
}

void Connection::stop() {
  _running = false;
}

void Connection::set_input(Input::Ptr val) {
  if (_input != val) {
    _input = val;
    KeyMaster_instance()->changed();
  }
}

void Connection::set_output(Output::Ptr val) {
  if (_output != val) {
    _output = val;
    KeyMaster_instance()->changed();
  }
}

void Connection::set_input_chan(int val) {
  if (_input_chan != val) {
    _input_chan = val;
    KeyMaster_instance()->changed();
  }
}

void Connection::set_output_chan(int val) {
  if (_output_chan != val) {
    _output_chan = val;
    KeyMaster_instance()->changed();
  }
}

void Connection::set_program_bank_msb(int val) {
  if (_prog.bank_msb != val) {
    _prog.bank_msb = val;
    KeyMaster_instance()->changed();
  }
}

void Connection::set_program_bank_lsb(int val) {
  if (_prog.bank_lsb != val) {
    _prog.bank_lsb = val;
    KeyMaster_instance()->changed();
  }
}

void Connection::set_program_prog(int val) {
  if (_prog.prog != val) {
    _prog.prog = val;
    KeyMaster_instance()->changed();
  }
}

void Connection::set_zone_low(int val) {
  if (_zone.low != val) {
    _zone.low = val;
    KeyMaster_instance()->changed();
  }
}

void Connection::set_zone_high(int val) {
  if (_zone.high != val) {
    _zone.high = val;
    KeyMaster_instance()->changed();
  }
}

void Connection::set_xpose(int val) {
  if (_xpose != val) {
    _xpose = val;
    KeyMaster_instance()->changed();
  }
}

void Connection::set_velocity_curve(Curve *val) {
  if (_velocity_curve != val) {
    _velocity_curve = val;
    KeyMaster_instance()->changed();
  }
}

void Connection::set_running(bool val) {
  if (_running != val) {
    _running = val;
    KeyMaster_instance()->changed();
  }
}

// Replaces controller mapping, deleting existing one if it exists.
void Connection::set_cc_map(int cc_num, Controller *val) {
  if (_cc_maps[cc_num] != val) {
    Controller *old = _cc_maps[cc_num];
    if (old != nullptr)
      delete old;

    _cc_maps[cc_num] = val;
    KeyMaster_instance()->changed();
  }
}

// Returns the channel for the initial bank/program change messages. If we
// can't determine that (both input and output channels are
// CONNECTION_ALL_CHANNELS) then return CONNECTION_ALL_CHANNELS.
int Connection::program_change_send_channel() {
  if (_output_chan != CONNECTION_ALL_CHANNELS)
    return _output_chan;
  return _input_chan;
}

// Call this when a Connection is being edited so that it can restart itself
// if it is _running.
void Connection::begin_changes() {
  _changing_was_running = is_running();
  if (_changing_was_running)
    stop();
}

// Call this when done making changes so the Connection can restart itself
// if it was _running.
void Connection::end_changes() {
  if (_changing_was_running) {
    start();
    _changing_was_running = false;
  }
}

// Takes a MIDI message `msg` from an input, processes it, and sends it to
// an output (unless it's been filtered out).
void Connection::midi_in(Input::Ptr input, const MidiMessage& msg) {
  if (!_running || _input == nullptr || _input != input || _output == nullptr)
    return;

  const uint8 *data = msg.getRawData();
  uint8 status = data[0];

  // See if the message should even be processed, or if we should stop here.
  if (!input_channel_ok(msg))
    return;
  if (_message_filter.filter_out(status, data[1])) {
    // If this is a sysex, send any realtime bytes inside it
    if (msg.isSysEx()) {
      const uint8 *sysex_data = msg.getSysExData();
      for (int i = 0; i < msg.getSysExDataSize(); ++i) {
        uint8 byte = sysex_data[i];
        if (is_realtime(byte))
          midi_out(MidiMessage(byte));
      }
    }
    return;
  }

  MidiMessage cc_msg;
  Controller *cc;

  int high_nibble = status & 0xf0;
  switch (high_nibble) {
  case NOTE_ON: case NOTE_OFF: case POLY_PRESSURE:
    if (inside_zone(data[1])) {
      int note_num = data[1] + _xpose;
      int velocity = data[2];
      if (_velocity_curve != nullptr)
        velocity = _velocity_curve->curve[velocity];
      if (note_num >= 0 && note_num <= 127) {
        if (_output_chan != CONNECTION_ALL_CHANNELS)
          status = uint8(high_nibble + _output_chan);
        midi_out(MidiMessage(status, note_num, velocity));
      }
    }
    break;
  case CONTROLLER:
    cc = _cc_maps[data[1]];
    if (cc != nullptr) {
      cc_msg = cc->process(msg, _output_chan);
      if (!mm_equal(cc_msg, EMPTY_MESSAGE)) // don't send
        midi_out(cc_msg);
    }
    else {
      if (_output_chan != CONNECTION_ALL_CHANNELS)
        status = uint8(high_nibble + _output_chan);
      midi_out(MidiMessage(status, data[1], data[2]));
    }
    break;
  case PROGRAM_CHANGE: case CHANNEL_PRESSURE:
    if (_output_chan != CONNECTION_ALL_CHANNELS)
      status = uint8(high_nibble + _output_chan);
    midi_out(MidiMessage(status, data[1]));
    break;
  case PITCH_BEND:
    if (_output_chan != CONNECTION_ALL_CHANNELS)
      status = uint8(high_nibble + _output_chan);
    midi_out(MidiMessage(status, data[1], data[2]));
    break;
  default:
    midi_out(msg);
    break;
  }
}

void Connection::set_controller(Controller *controller) {
  int cc_num = controller->cc_num();
  if (_cc_maps[cc_num] != controller) {
    if (_cc_maps[cc_num] != nullptr)
      remove_cc_num(cc_num);
    _cc_maps[cc_num] = controller;
    KeyMaster_instance()->changed();
  }
}

void Connection::remove_cc_num(int cc_num) {
  if (_cc_maps[cc_num] != nullptr) {
    delete _cc_maps[cc_num];
    _cc_maps[cc_num] = nullptr;
    KeyMaster_instance()->changed();
  }
}

// Returns `true` if any one of the following are true:
// - we accept any input channel
// - it's a system message, not a channel message
// - the input channel matches our selected `input_chan`
int Connection::input_channel_ok(const MidiMessage &msg) {
  if (_input_chan == CONNECTION_ALL_CHANNELS || msg.isSysEx())
    return true;

  int channel = msg.getChannel();
  if (channel == 0)             // not a channel message
    return true;
  return _input_chan == channel - 1;
}

int Connection::inside_zone(int note) {
  return note >= _zone.low && note <= _zone.high;
}

void Connection::midi_out(MidiMessage *message) {
  if (_output != nullptr)
    _output->midi_out(*message);
}

void Connection::midi_out(MidiMessage message) {
  if (_output != nullptr)
    _output->midi_out(message);
}
