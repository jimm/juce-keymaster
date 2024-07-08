#include <stdlib.h>
#include "consts.h"
#include "keymaster.h"
#include "cursor.h"

#define PATCH_STOP  {if (_cursor->patch() != nullptr) _cursor->patch()->stop();}
#define PATCH_START {update_clock(); if (_cursor->patch() != nullptr) _cursor->patch()->start();}

#define PRD Logger::writeToLog
#define FS String::formatted

static KeyMaster *km_instance = nullptr;

KeyMaster *KeyMaster_instance() {
  return km_instance;
}

KeyMaster *create_KeyMaster() {
  KeyMaster *old_km = KeyMaster_instance();
  bool testing = old_km != nullptr && old_km->is_testing();

  KeyMaster *km = new KeyMaster();
  km->set_testing(testing);

  if (old_km != nullptr)
    delete old_km;

  km->initialize();
  km->start();
  return km;
}

// ================ allocation ================

KeyMaster::KeyMaster(bool testing)
  : _cursor(0), _clock(), _running(false), _testing(testing)
{
  initialiseWithDefaultDevices(0, 0); // AudioDeviceManager
  load_instruments();
  _set_lists.add(new SetList(UNDEFINED_ID, "All Songs"));
  _cursor = new Cursor(this);
  km_instance = this;
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

  _inputs.clear();
  _outputs.clear();
}

// ================ accessors ================

void KeyMaster::add_message(MessageBlock *message) {
  _messages.add(message);
}

void KeyMaster::remove_message(MessageBlock *message) {
  _messages.removeFirstMatchingValue(message);
  delete message;
}

void KeyMaster::add_curve(Curve *curve) {
  _curves.add(curve);
}

void KeyMaster::remove_curve(Curve *curve) {
  _curves.removeFirstMatchingValue(curve);
  delete curve;
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
}

void KeyMaster::remove_trigger(Trigger *trigger) {
  _triggers.removeFirstMatchingValue(trigger);
  delete trigger;
}

void KeyMaster::add_set_list(SetList *set_list) {
  _set_lists.add(set_list);
}

void KeyMaster::remove_set_list(SetList *set_list) {
  _set_lists.removeFirstMatchingValue(set_list);
  delete set_list;
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
  PATCH_STOP;
  _running = false;
}

void KeyMaster::handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message) {
  if (message.isActiveSense())
    return;

  // TODO listen for program changes and jump to song
  for (auto &trigger : _triggers)
    trigger->signal_message(source, message);
  if (_cursor != nullptr)       // we might get MIDI before we're fully constructed
    _cursor->patch()->midi_in(source, message);
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

void KeyMaster::initialize() {
  generate_default_curves(this->_curves);
  create_songs();
}

void KeyMaster::load_instruments() {
  if (_testing)
    return;

  _inputs.clear();
  for (auto info : MidiInput::getAvailableDevices())
    _inputs.add(new MidiInputEntry(info, this));

  _outputs.clear();
  for (auto info : MidiOutput::getAvailableDevices())
    _outputs.add(new MidiOutputEntry(info));
}

void KeyMaster::create_songs() {
  for (auto& input : _inputs) {

    // this input to each individual output
    for (auto& output : _outputs) {
      String name = input->device->getName() + " -> " + output->device->getName();
      Song *song = new Song(UNDEFINED_ID, name);
      all_songs()->add_song(song);

      Patch *patch = new Patch(UNDEFINED_ID, name);
      song->add_patch(patch);

      Connection *conn = new Connection(
        UNDEFINED,
        input->info, CONNECTION_ALL_CHANNELS,
        output->device, CONNECTION_ALL_CHANNELS);
      patch->add_connection(conn);

    }

    if (_outputs.size() > 1) {
      // one more song: this input to all _outputs at once
      String name = input->device->getName() + " -> all outputs";
      Song *song = new Song(UNDEFINED_ID, name);
      all_songs()->add_song(song);

      Patch *patch = new Patch(UNDEFINED_ID, name);
      song->add_patch(patch);

      for (auto& output : _outputs) {
        Connection *conn = new Connection(
          UNDEFINED,
          input->info, CONNECTION_ALL_CHANNELS,
          output->device, CONNECTION_ALL_CHANNELS);
        patch->add_connection(conn);
      }
    }
  }
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
  MidiMessageSequence buf;

  if (send_notes_off) {
    MidiMessageSequence buf;
    for (int i = 0; i < 16; ++i) {
      for (int j = 0; j < 128; ++j)
        buf.addEvent(MidiMessage::noteOff(i, j), 0);
    }
  }
  else {
    for (int i = 0; i < 16; ++i)
      buf.addEvent(MidiMessage::controllerEvent(i, CM_ALL_NOTES_OFF, 0), 0);
  }

  for (auto &msg : buf)
    for (auto &out : _outputs)
      out->device->sendMessageNow(msg->message);
}

// ================ helpers ================

void KeyMaster::sort_all_songs() {
  auto sorter = NameableSorter();
  all_songs()->songs().sort(sorter, false);
}
