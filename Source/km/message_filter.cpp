#include "consts.h"
#include "message_filter.h"

// true means allow, false means filter out

MessageFilter::MessageFilter()
  :
  _note(true),                  // both on and off
  _poly_pressure(true),
  _chan_pressure(true),
  _program_change(false),       // filter out by default
  _pitch_bend(true),
  _controller(true),
  _song_pointer(true),
  _song_select(true),
  _tune_request(true),
  _sysex(false),                // filter out by default
  _clock(true),
  _start_continue_stop(true),
  _system_reset(true)
{
}

// Return true if the message for this status should be filtered out.
bool MessageFilter::filter_out(int status, int data1) {
  if (status < 0xf0)
    status = status & 0xf0;
  switch (status) {
    case NOTE_OFF:
    case NOTE_ON:
      return !_note;
    case POLY_PRESSURE:
      return !POLY_PRESSURE;
    case CONTROLLER:
      if (data1 == CC_BANK_SELECT_MSB || data1 == CC_BANK_SELECT_LSB)
        return !_program_change;
      return !_controller;
    case PROGRAM_CHANGE:
      return !_program_change;
    case CHANNEL_PRESSURE:
      return !_chan_pressure;
    case PITCH_BEND:
      return !_pitch_bend;
    case SYSEX:
    case EOX:
      return !_sysex;
    case SONG_POINTER:
      return !_song_pointer;
    case SONG_SELECT:
      return !_song_select;
    case TUNE_REQUEST:
      return !_tune_request;
    case CLOCK:
      return !_clock;
    case START:
    case CONTINUE:
    case STOP:
      return !_start_continue_stop;
    case ACTIVE_SENSE:
      // When we initialize PortMidi we always filter out all active sensing
      // messages. This does the same thing.
      return true;
    case SYSTEM_RESET:
      return !_system_reset;
  default:
    // error, but don't prevent wierdness from happening I suppose
    return false;
  }
}
