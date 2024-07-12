#pragma once

#include "consts.h"
#include "instrument.h"

class Patch;

class Input : public Instrument {
public:

  using Ptr = ReferenceCountedObjectPtr<Input>;

  static Ptr find_by_id(String identifier);

  Input(MidiDeviceInfo info, MidiInputCallback *listener);

  Patch *patch_for_message(const MidiMessage &msg);

private:
  std::unique_ptr<MidiInput> device;
  Patch *note_off_patches[MIDI_CHANNELS][NOTES_PER_CHANNEL];
  Patch *sustain_off_patches[MIDI_CHANNELS];
};
