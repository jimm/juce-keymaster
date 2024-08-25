#include <stdlib.h>
#include "consts.h"
#include "connection.h"
#include "keymaster.h"

#define is_status(b) (((b) & 0x80) == 0x80)
#define is_system(b) ((b) >= SYSEX)
#define is_realtime(b) ((b) >= 0xf8)

Connection::Connection(DBObjID id, Input::Ptr input, int in_chan, Output::Ptr output, int out_chan)
  : DBObj(id),
    _input(input), _output(output),
    _input_chan(in_chan), _output_chan(out_chan),
    _xpose(0), _velocity_curve(nullptr),
    _processing_sysex(false),
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
    _processing_sysex(false),
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

  if (_output != nullptr) {
    int chan = program_change_send_channel();
    if (chan != CONNECTION_ALL_CHANNELS) {
      if (_prog.bank_msb >= 0)
        midi_out(MidiMessage::controllerEvent(chan+1, CC_BANK_SELECT_MSB, _prog.bank_msb));
      if (_prog.bank_lsb >= 0)
        midi_out(MidiMessage::controllerEvent(chan+1, CC_BANK_SELECT_LSB, _prog.bank_lsb));
      if (_prog.prog >= 0)
        midi_out(MidiMessage::programChange(chan+1, _prog.prog));
    }
  }

  _processing_sysex = false;
  _running = true;
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

void Connection::set_processing_sysex(bool val) {
  if (_processing_sysex != val) {
    _processing_sysex = val;
    KeyMaster_instance()->changed();
  }
}

void Connection::set_running(bool val) {
  if (_running != val) {
    _running = val;
    KeyMaster_instance()->changed();
  }
}

void Connection::set_cc_map(int cc_num, Controller *val) {
  if (_cc_maps[cc_num] != val) {
    _cc_maps[cc_num] = val;
    KeyMaster_instance()->changed();
  }
}

// Returns the channel that we should send the initial bank/program change
// messages. If we can't determine that (both input and output channels are
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
  if (_input == nullptr || _input != input || _output == nullptr)
    return;

  const juce::uint8 *data = msg.getRawData();
  juce::uint8 status = data[0];

  // See if the message should even be processed, or if we should stop here.
  if (!input_channel_ok(msg))
    return;

  if (msg.isSysEx())
    _processing_sysex = true;

  // Grab filter boolean for this status. If we're inside a sysex message,
  // we need use SYSEX as the filter status, not the first byte of this
  // message.
  bool filter_this_status = _message_filter.filter_out(_processing_sysex ? SYSEX : status, data[1]);

  // Return if we're filtering this message, exept if we're starting or in
  // sysex. In that case we need to keep going because we need to process
  // realtime bytes within the sysex.
  if (!_processing_sysex && filter_this_status)
    return;

  // If this is a sysex message, we may or may not filter it out. In any
  // case we pass through any realtime bytes in the sysex message.
  if (_processing_sysex) {
    if (status == EOX || data[1] == EOX || data[2] == EOX || data[3] == EOX ||
        // non-realtime status byte
        (is_status(status) && status < 0xf8 && status != SYSEX))
      _processing_sysex = false;

    if (!filter_this_status) {
      midi_out(msg);
      return;
    }

    // If any of the bytes are realtime bytes AND if we are filtering out
    // sysex, send them anyway.
    if (is_realtime(status) && !_message_filter.filter_out(status, 0)) {
      midi_out(MidiMessage(status));
    }
    for (int i = 1; i <= 3; ++i)
      if (is_realtime(data[i]) && !_message_filter.filter_out(data[i], 0))
        midi_out(MidiMessage(data[i]));
    return;
  }

  MidiMessage cc_msg;
  Controller *cc;

  int high_nibble = status & 0xf0;
  switch (high_nibble) {
  case NOTE_ON: case NOTE_OFF: case POLY_PRESSURE:
    if (inside_zone(data[1])) {
      juce::uint8 buf[3] = {data[0], data[1], data[2]};
      buf[1] += _xpose;
      if (_velocity_curve != nullptr)
        buf[2] = _velocity_curve->curve[data[2]];
      if (data[1] >= 0 && data[1] <= 127) {
        if (_output_chan != CONNECTION_ALL_CHANNELS)
          status = uint8(high_nibble + _output_chan + 1); // MidiMessage chans 1-16
        midi_out(MidiMessage(status, buf[1], buf[2]));
      }
    }
    break;
  case CONTROLLER:
    cc = _cc_maps[data[1]];
    if (cc != nullptr) {
      cc_msg = cc->process(msg, _output_chan);
      if (!cc_msg.isActiveSense()) // ACTIVE_SENSE means don't send
        midi_out(cc_msg);
    }
    else {
      if (_output_chan != CONNECTION_ALL_CHANNELS)
        status = uint8(high_nibble + _output_chan + 1); // MidiMessage chans 1-16
      midi_out(MidiMessage(status, data[1], data[2]));
    }
    break;
  case PROGRAM_CHANGE: case CHANNEL_PRESSURE: case PITCH_BEND:
    if (_output_chan != CONNECTION_ALL_CHANNELS)
      status = uint8(high_nibble + _output_chan + 1); // MidiMessage chans 1-16
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
  if (_input_chan == CONNECTION_ALL_CHANNELS || _processing_sysex)
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
