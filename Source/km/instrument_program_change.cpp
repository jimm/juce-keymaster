#include "instrument_program_change.h"
#include "consts.h"

InstrumentProgramChange::InstrumentProgramChange()
  : output(nullptr), channel(1),
    bank_msb(UNDEFINED), bank_lsb(UNDEFINED), prog(UNDEFINED)
{}

void InstrumentProgramChange::send() {
  if (output == nullptr)
    return;
  MidiMessage msg;
  if (bank_msb != UNDEFINED) {
    msg = MidiMessage::controllerEvent(channel, CC_BANK_SELECT_MSB, bank_msb);
    output->midi_out(msg);
  }
  if (bank_lsb != UNDEFINED) {
    msg = MidiMessage::controllerEvent(channel, CC_BANK_SELECT_LSB, bank_lsb);
    output->midi_out(msg);
  }
  if (prog != UNDEFINED) {
    msg = MidiMessage::programChange(channel, prog);
    output->midi_out(msg);
  }
}
