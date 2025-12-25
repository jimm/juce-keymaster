#include <stdlib.h>
#include "consts.h"
#include "device_manager.h"
#include "keymaster.h"
#include "cursor.h"

#define PATCH_STOP  {if (_cursor->patch() != nullptr) _cursor->patch()->stop();}
#define PATCH_START {update_clock(); if (_cursor->patch() != nullptr) _cursor->patch()->start();}

static KeyMaster *km_instance = nullptr;
const MidiMessage EMPTY_MESSAGE { EMPTY_MESSAGE_BYTE };

KeyMaster *KeyMaster_instance() {
  return km_instance;
}

// Does not stop existing instance or delete it or anything.
void set_KeyMaster_instance(KeyMaster *km) {
  km_instance = km;
}

// ================ allocation ================

KeyMaster::KeyMaster(DeviceManager &dev_mgr, bool testing)
  : _device_manager(dev_mgr),
#ifndef JUCE_UNIT_TESTS
    _doc(nullptr),
#endif
    _cursor(nullptr),
    _clock(),
    _running(false),
    _testing(testing)
{
  _set_lists.add(new SetList(UNDEFINED_ID, "All Songs"));
  _cursor = new Cursor(this);
  set_KeyMaster_instance(this);

  // Take this opportunity to tell the device manager to forget about any non-attached devices.
  _device_manager.update_devices();
}

KeyMaster::~KeyMaster() {
  stop();

  if (km_instance == this)
    km_instance = nullptr;

  for (auto& t : _triggers)
    delete t;
  for (auto& song : all_songs()->songs())
    delete song;
  for (auto& set_list : _set_lists)
    delete set_list;
  for (auto& msg : _messages)
    delete msg;
  for (auto& curve : _curves)
    delete curve;
}

// ================ accessors ================

void KeyMaster::add_message(MessageBlock *message) {
  _messages.add(message);
  changed();
}

void KeyMaster::remove_message(MessageBlock *message) {
  _messages.removeFirstMatchingValue(message);
  delete message;
  changed();
}

void KeyMaster::add_curve(Curve *curve) {
  _curves.add(curve);
  changed();
}

void KeyMaster::remove_curve(Curve *curve) {
  _curves.removeFirstMatchingValue(curve);
  delete curve;
  changed();
}

Curve * KeyMaster::curve_with_name(const String &name) {
  for (auto &curve : _curves)
    if (curve->name() == name)
      return curve;
  return nullptr;
}

Curve * KeyMaster::curve_with_id(DBObjID id) {
  for (auto &curve : _curves)
    if (curve->id() == id)
      return curve;
  return nullptr;
}

void KeyMaster::add_trigger(Trigger *trigger) {
  _triggers.add(trigger);
  changed();
}

void KeyMaster::remove_trigger(Trigger *trigger) {
  _triggers.removeFirstMatchingValue(trigger);
  delete trigger;
  changed();
}

void KeyMaster::add_set_list(SetList *set_list) {
  _set_lists.add(set_list);
  changed();
}

void KeyMaster::remove_set_list(SetList *set_list) {
  _set_lists.removeFirstMatchingValue(set_list);
  delete set_list;
  changed();
}

// ================ running ================

void KeyMaster::start() {
  if (_running)
    return;

  _cursor->init();
  _running = true;
  PATCH_START;
}

void KeyMaster::stop() {
  if (!_running)
    return;

  stop_clock();
  send_pending_offs();
  PATCH_STOP;
  _running = false;
}

void KeyMaster::send_pending_offs() {
  for (auto input : _device_manager.inputs())
    input->send_pending_offs();
}

// ================ clock ================

void KeyMaster::update_clock() {
  Song *curr_song = _cursor->song();
  if (curr_song == nullptr || !curr_song->clock_on_at_start())
    stop_clock();
  else {
    set_clock_bpm(curr_song->bpm());
    start_clock();
  }
}

// ================ initialization ================

// Call this when you've created a KeyMaster instance that will not then
// read its data from a file.
void KeyMaster::initialize() {
  generate_default_curves(this->_curves);
}

// ================ movement ================

void KeyMaster::next_patch() {
  PATCH_STOP;
  _cursor->next_patch();
  PATCH_START;
}

void KeyMaster::prev_patch() {
  PATCH_STOP;
  _cursor->prev_patch();
  PATCH_START;
}

void KeyMaster::next_song() {
  PATCH_STOP;
  _cursor->next_song();
  PATCH_START;
}

void KeyMaster::prev_song() {
  PATCH_STOP;
  _cursor->prev_song();
  PATCH_START;
}

// ================ going places ================

void KeyMaster::goto_song(Song *song) {
  PATCH_STOP;
  _cursor->goto_song(song);
  PATCH_START;
}

void KeyMaster::goto_patch(Patch *patch) {
  PATCH_STOP;
  _cursor->goto_patch(patch);
  PATCH_START;
}

void KeyMaster::goto_song(const String &name_regex) {
  PATCH_STOP;
  _cursor->goto_song(name_regex);
  PATCH_START;
}

void KeyMaster::goto_set_list(const String &name_regex) {
  PATCH_STOP;
  _cursor->goto_set_list(name_regex);
  PATCH_START;
}

void KeyMaster::jump_to_set_list_index(int i) {
  if (i == _cursor->set_list_index)
    return;

  PATCH_STOP;
  _cursor->jump_to_set_list_index(i);
  PATCH_START;
}

void KeyMaster::jump_to_song_index(int i) {
  if (i == _cursor->song_index)
    return;

  PATCH_STOP;
  _cursor->jump_to_song_index(i);
  PATCH_START;
}

void KeyMaster::jump_to_patch_index(int i) {
  if (i == _cursor->patch_index)
    return;

  PATCH_STOP;
  _cursor->jump_to_patch_index(i);
  PATCH_START;
}

// ================ doing things ================

void KeyMaster::panic(bool send_notes_off) {
  Array<MidiMessage> buf;

  if (send_notes_off) {
    for (int juce_chan = 1; juce_chan <= MIDI_CHANNELS; ++juce_chan) {
      for (int note = 0; note < 128; ++note)
        buf.add(MidiMessage::noteOff(juce_chan, note));
    }
  }
  else {
    for (int juce_chan = 1; juce_chan <= MIDI_CHANNELS; ++juce_chan)
      buf.add(MidiMessage::controllerEvent(juce_chan, CM_ALL_NOTES_OFF, 0));
  }

  for (auto &msg : buf)
    for (auto &out : _device_manager.outputs())
      out->midi_out(msg);
}
