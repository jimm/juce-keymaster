#pragma once

#include <JuceHeader.h>
#include "consts.h"
#include "instrument.h"

class Patch;

class Input : public Instrument {
public:

  using Ptr = ReferenceCountedObjectPtr<Input>;

  static Ptr find_by_id(String identifier);

  Input(MidiDeviceInfo info, MidiInputCallback *listener);

  Patch *patch_for_message(MidiInput *source, const MidiMessage &msg);
  void send_pending_offs();

private:
  std::unique_ptr<MidiInput> device;
  Patch *note_off_patches[MIDI_CHANNELS][NOTES_PER_CHANNEL];
  Patch *sustain_off_patches[MIDI_CHANNELS];
};
