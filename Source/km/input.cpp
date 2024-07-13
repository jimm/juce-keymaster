#include "keymaster.h"
#include "input.h"

Input::Input(MidiDeviceInfo info, MidiInputCallback *listener)
  : Instrument(info), device(MidiInput::openDevice(info.identifier, listener))
{
  for (int chan = 0; chan < MIDI_CHANNELS; ++chan) {
    sustain_off_patches[chan] = nullptr;
    for (int note = 0; note < NOTES_PER_CHANNEL; ++note)
      note_off_patches[chan][note] = nullptr;
  }
  device->start();
}

Input::Ptr Input::find_by_id(String identifier) {
  for (auto &input : KeyMaster_instance()->inputs())
    if (input->identifier() == identifier)
      return input;
  return nullptr;
}

// Note off and sustain off messages must be sent to the same patch as the
// corresponding note on or sustain on. For the on messages messages we
// store the current patch and for off messages we return it.
//
// We return the current patch for all other messages.
Patch * Input::patch_for_message(MidiInput *source, const MidiMessage &msg) {
  Patch *curr_patch = KeyMaster_instance()->cursor()->patch();
  int channel = msg.getChannel(); // 1-16 or 0 if not a channel message

  // Handle non-channel bytes as quickly as possible
  if (channel == 0)
    return curr_patch;

  if (msg.isNoteOff(true)) {    // also checks if note on but velocity zero
    int note_num = msg.getNoteNumber();
    Patch *p = note_off_patches[channel-1][note_num];
    if (p) {
      note_off_patches[channel-1][note_num] = nullptr;
      return p;
    }
    return curr_patch;
  }

  if (msg.isNoteOn()) {
    int note_num = msg.getNoteNumber();
    note_off_patches[channel-1][note_num] = curr_patch;
    return curr_patch;
  }

  if (msg.isSustainPedalOff()) {
    Patch *p = sustain_off_patches[channel-1];
    if (p) {
      sustain_off_patches[channel-1] = nullptr;
      return p;
    }
    return curr_patch;
  }

  if (msg.isSustainPedalOn()) {
    sustain_off_patches[channel-1] = curr_patch;
    return curr_patch;
  }

  return curr_patch;
}

void Input::send_pending_offs() {
  for (int chan = 0; chan < MIDI_CHANNELS; ++chan) {
    if (sustain_off_patches[chan] != nullptr) {
      sustain_off_patches[chan]->midi_in(device.get(), MidiMessage::controllerEvent(chan, CC_SUSTAIN, 0));
      sustain_off_patches[chan] = nullptr;
    }
    for (int note = 0; note < NOTES_PER_CHANNEL; ++note) {
      if (note_off_patches[chan][note] != nullptr) {
        note_off_patches[chan][note]->midi_in(device.get(), MidiMessage::noteOff(chan, note, (uint8)64));
        note_off_patches[chan][note] = nullptr;
      }
    }
  }
}
