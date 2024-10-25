#include "keymaster.h"
#include "input.h"
#include "midi_message_learner.h"
#include "utils.h"

Input::Input() {
  initialize();
}

Input::Input(MidiDeviceInfo device_info, MidiInputCallback *listener)
  : Instrument(device_info), device(MidiInput::openDevice(info.identifier, listener))
{
  initialize();
}

Input::~Input() {
  stop();
}

void Input::initialize() {
  for (int chan = 0; chan < MIDI_CHANNELS; ++chan) {
    sustain_off_patches[chan] = nullptr;
    for (int note = 0; note < 128; ++note)
      note_off_patches[chan][note] = nullptr;
  }
  start();
}

void Input::start() {
  if (device)
    device->start();
}

void Input::stop() {
  if (device) {
    send_pending_offs();
    device->stop();
  }
}

void Input::midi_in(const MidiMessage &message) {
  if (message.isActiveSense())
    return;

  KeyMaster *km = KeyMaster_instance();
  if (km == nullptr || km->cursor() == nullptr) // we might get MIDI before we're fully constructed
    return;

  for (auto &trigger : km->triggers())
    trigger->signal_message(message);

  // Let the input tell us which patch to use. By default it's the current
  // patch, but if this is a note off or sustain off then we need to send
  // that to the same patch as that used by the corresponding note on or
  // sustain on.
  Patch *p = patch_for_message(message);
  if (p != nullptr)
    p->midi_in(this, message);

  if (!_listeners.isEmpty())
    for (auto listener : _listeners)
      listener->midi_input(name(), message);
}

// Note off and sustain off messages must be sent to the same patch as the
// corresponding note on or sustain on. For the on messages messages we
// store the current patch and for off messages we return it.
//
// We return the current patch for all other messages.
Patch * Input::patch_for_message(const MidiMessage &msg) {
  Patch *curr_patch = KeyMaster_instance()->cursor()->patch();
  int channel = msg.getChannel(); // 1-16 or 0 if not a channel message

  // Handle non-channel bytes as quickly as possible
  if (channel == 0)
    return curr_patch;

  --channel;

  if (msg.isNoteOff(true)) {    // also checks if note on but velocity zero
    int note_num = msg.getNoteNumber();
    Patch *p = note_off_patches[channel][note_num];
    if (p) {
      note_off_patches[channel][note_num] = nullptr;
      return p;
    }
    return curr_patch;
  }

  if (msg.isNoteOn()) {
    int note_num = msg.getNoteNumber();
    note_off_patches[channel][note_num] = curr_patch;
    return curr_patch;
  }

  if (msg.isSustainPedalOff()) {
    Patch *p = sustain_off_patches[channel];
    if (p) {
      sustain_off_patches[channel] = nullptr;
      return p;
    }
    return curr_patch;
  }

  if (msg.isSustainPedalOn()) {
    sustain_off_patches[channel] = curr_patch;
    return curr_patch;
  }

  return curr_patch;
}

void Input::send_pending_offs() {
  for (int chan = 0; chan < MIDI_CHANNELS; ++chan) {
    if (sustain_off_patches[chan] != nullptr) {
      sustain_off_patches[chan]->midi_in(this, MidiMessage::controllerEvent(JCH(chan), CC_SUSTAIN, 0));
      sustain_off_patches[chan] = nullptr;
    }
    for (int note = 0; note < NOTES_PER_CHANNEL; ++note) {
      if (note_off_patches[chan][note] != nullptr) {
        note_off_patches[chan][note]->midi_in(this, MidiMessage::noteOff(JCH(chan), note, (uint8)64));
        note_off_patches[chan][note] = nullptr;
      }
    }
  }
}

// Sends all pending offs meant for patch p.
void Input::patch_being_deleted(Patch *p) {
  for (int chan = 0; chan < MIDI_CHANNELS; ++chan) {
    auto patch = sustain_off_patches[chan];
    if (patch == p) {
      p->midi_in(this, MidiMessage::controllerEvent(JCH(chan), CC_SUSTAIN, 0));
      sustain_off_patches[chan] = nullptr;
    }
    for (int note = 0; note < NOTES_PER_CHANNEL; ++note) {
      patch = note_off_patches[chan][note];
      if (patch == p) {
        p->midi_in(this, MidiMessage::noteOff(JCH(chan), note, (uint8)64));
        note_off_patches[chan][note] = nullptr;
      }
    }
  }
}

