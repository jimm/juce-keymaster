#pragma once

#include <JuceHeader.h>
#include "consts.h"
#include "instrument.h"
#include "midi_message_learner.h"

class Patch;
class MidiMessageLearner;

class Input : public Instrument {
public:

  using Ptr = ReferenceCountedObjectPtr<Input>;

  Input();                      // for testing
  Input(MidiDeviceInfo device_info, MidiInputCallback *listener);
  virtual ~Input() override;

  virtual void start() override;
  virtual void stop() override;
  bool is_running() override { return (bool)device; }

  // Incoming messages are sent to the learner until stop_learning is called
  void start_learning(MidiMessageLearner *learner) { message_learner = learner; }
  void stop_learning() { message_learner = nullptr; }

  void midi_in(const MidiMessage &msg);

  void send_pending_offs();

  void patch_being_deleted(Patch *p);

private:
  std::unique_ptr<MidiInput> device;
  Patch *note_off_patches[MIDI_CHANNELS][NOTES_PER_CHANNEL];
  Patch *sustain_off_patches[MIDI_CHANNELS];
  MidiMessageLearner *message_learner;

  void initialize();
  Patch *patch_for_message(const MidiMessage &msg);
};
