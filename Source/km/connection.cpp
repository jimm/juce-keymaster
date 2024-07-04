#include <stdlib.h>
#include "consts.h"
#include "connection.h"

#define is_status(b) (((b) & 0x80) == 0x80)
#define is_system(b) ((b) >= SYSEX)
#define is_realtime(b) ((b) >= 0xf8)

Connection::Connection(DBObjID id, MidiInputEntry::Ptr in, int in_chan, MidiOutputEntry::Ptr out, int out_chan)
  : DBObj(id),
    _input(in), _output(out),
    _input_chan(in_chan), _output_chan(out_chan),
    _xpose(0), _velocity_curve(nullptr),
    _processing_sysex(false),
    _running(false), _changing_was_running(false)
{
  _prog.bank_msb = _prog.bank_lsb = _prog.prog = UNDEFINED;
  _zone.low = 0;
  _zone.high = 127;
  for (int i = 0; i < 128; ++i)
    _cc_maps[i] = nullptr;
}

Connection::~Connection() {
  for (int i = 0; i < 128; ++i)
    if (_cc_maps[i] != nullptr)
      delete _cc_maps[i];
}

void Connection::start() {
  if (_running)
    return;

  // The program output channel is either the output channel if specified or
  // else the output channel if specified. If they are both ALL then we
  // don't know which channel to send the program change on, so we don't
  // send one.
  int chan = program_change_send_channel();
  if (chan != CONNECTION_ALL_CHANNELS) {
    if (_prog.bank_msb >= 0)
      _output->device->sendMessageNow(MidiMessage::controllerEvent(chan, CC_BANK_SELECT_MSB, _prog.bank_msb));
    if (_prog.bank_lsb >= 0)
      _output->device->sendMessageNow(MidiMessage::controllerEvent(chan, CC_BANK_SELECT_LSB, _prog.bank_lsb));
    if (_prog.prog >= 0)
      _output->device->sendMessageNow(MidiMessage::programChange(chan, _prog.prog));
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
void Connection::midi_in(MidiInput* source, const MidiMessage& msg) {
  const juce::uint8 *data = msg.getRawData();
  juce::uint8 status = data[0];

  // See if the message should even be processed, or if we should stop here.
  if (!input_channel_ok(status))
    return;

  int high_nibble = status & 0xf0;

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
    if (is_realtime(data[1]) && !_message_filter.filter_out(data[1], 0))
      midi_out(MidiMessage(data[1]));
    if (is_realtime(data[2]) && !_message_filter.filter_out(data[2], 0))
      midi_out(MidiMessage(data[2]));
    if (is_realtime(data[3]) && !_message_filter.filter_out(data[3], 0))
      midi_out(MidiMessage(data[3]));
    return;
  }

  MidiMessage cc_msg;
  Controller *cc;

  switch (high_nibble) {
  case NOTE_ON: case NOTE_OFF: case POLY_PRESSURE:
    if (inside_zone(data[1])) {
      juce::uint8 buf[3] = {data[0], data[1], data[2]};
      buf[1] += _xpose;
      if (_velocity_curve != nullptr)
        buf[2] = _velocity_curve->curve[data[2]];
      if (data[1] >= 0 && data[1] <= 127) {
        if (_output_chan != CONNECTION_ALL_CHANNELS)
          status = high_nibble + _output_chan;
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
        status = high_nibble + _output_chan;
      midi_out(MidiMessage(status, data[1], data[2]));
    }
    break;
  case PROGRAM_CHANGE: case CHANNEL_PRESSURE: case PITCH_BEND:
    if (_output_chan != CONNECTION_ALL_CHANNELS)
      status = high_nibble + _output_chan;
    midi_out(MidiMessage(status, data[1], data[2]));
    break;
  default:
    midi_out(msg);
    break;
  }
}

void Connection::set_controller(Controller *controller) {
  int cc_num = controller->cc_num();
  if (_cc_maps[cc_num] != nullptr && _cc_maps[cc_num] != controller)
    remove_cc_num(cc_num);
  _cc_maps[cc_num] = controller;
}

void Connection::remove_cc_num(int cc_num) {
  if (_cc_maps[cc_num] != nullptr) {
    delete _cc_maps[cc_num];
    _cc_maps[cc_num] = nullptr;
  }
}

// Returns `true` if any one of the following are true:
// - we accept any input channel
// - it's a system message, not a channel message
// - the input channel matches our selected `input_chan`
int Connection::input_channel_ok(int status) {
  if (_input_chan == CONNECTION_ALL_CHANNELS || _processing_sysex)
    return true;

  return is_system(status) || _input_chan == (status & 0x0f);
}

int Connection::inside_zone(int note) {
  return note >= _zone.low && note <= _zone.high;
}

void Connection::midi_out(MidiMessage *message) {
  _output->device->sendMessageNow(*message);
}

void Connection::midi_out(MidiMessage message) {
  _output->device->sendMessageNow(message);
}
