#include <stdlib.h>
#include "consts.h"
#include "keymaster.h"
#include "cursor.h"

#define PATCH_STOP  {if (_cursor->patch() != nullptr) _cursor->patch()->stop();}
#define PATCH_START {update_clock(); if (_cursor->patch() != nullptr) _cursor->patch()->start();}

static KeyMaster *km_instance = nullptr;

KeyMaster *KeyMaster_instance() {
  return km_instance;
}

// Does not stop existing instance or delete it or anything.
void set_KeyMaster_instance(KeyMaster *km) {
  km_instance = km;
}

// ================ allocation ================

KeyMaster::KeyMaster(bool testing)
  : _doc(nullptr), _cursor(0), _clock(), _running(false), _testing(testing)
{
  initialiseWithDefaultDevices(0, 0); // AudioDeviceManager
  load_instruments();
  _set_lists.add(new SetList(UNDEFINED_ID, "All Songs"));
  _cursor = new Cursor(this);
  set_KeyMaster_instance(this);
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

  _identifier_to_input.clear();
  _inputs.clear();
  _outputs.clear();
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

void KeyMaster::handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message) {
  if (message.isActiveSense())
    return;

  // TODO listen for program changes and jump to song
  for (auto &trigger : _triggers)
    trigger->signal_message(source, message);

  if (_cursor == nullptr) // we might get MIDI before we're fully constructed
    return;

  // Let the input tell us which patch to use. By default it's the current
  // patch, but if this is a note off or sustain off then we need to send
  // that to the same patch as that used by the corresponding note on or
  // sustain on.
  Patch *p = _identifier_to_input.contains(source->getIdentifier())
    ? _identifier_to_input[source->getIdentifier()]->patch_for_message(source, message)
    : _cursor->patch();

  p->midi_in(source, message);
}

void KeyMaster::send_pending_offs() {
  for (auto input : _inputs)
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

void KeyMaster::initialize() {
  generate_default_curves(this->_curves);
  create_songs();
}

void KeyMaster::load_instruments() {
  if (_testing)
    return;

  _inputs.clear();
  _identifier_to_input.clear();
  for (auto info : MidiInput::getAvailableDevices()) {
    Input::Ptr input = new Input(info, this);
    _inputs.add(input);
    _identifier_to_input.set(input->identifier(), input);
  }

  _outputs.clear();
  for (auto info : MidiOutput::getAvailableDevices())
    _outputs.add(new Output(info));
}

void KeyMaster::create_songs() {
  for (auto& input : _inputs) {

    // this input to each individual output
    for (auto& output : _outputs) {
      String name = input->name() + " -> " + output->name();
      Song *song = new Song(UNDEFINED_ID, name);
      all_songs()->add_song(song);

      Patch *patch = new Patch(UNDEFINED_ID, name);
      song->add_patch(patch);

      Connection *conn = new Connection(
        UNDEFINED,
        input, CONNECTION_ALL_CHANNELS,
        output, CONNECTION_ALL_CHANNELS);
      patch->add_connection(conn);

    }

    if (_outputs.size() > 1) {
      // one more song: this input to all _outputs at once
      String name = input->name() + " -> all outputs";
      Song *song = new Song(UNDEFINED_ID, name);
      all_songs()->add_song(song);

      Patch *patch = new Patch(UNDEFINED_ID, name);
      song->add_patch(patch);

      for (auto& output : _outputs) {
        Connection *conn = new Connection(
          UNDEFINED,
          input, CONNECTION_ALL_CHANNELS,
          output, CONNECTION_ALL_CHANNELS);
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
      out->midi_out(msg->message);
}

// ================ helpers ================

void KeyMaster::sort_all_songs() {
  auto sorter = NameableSorter();
  all_songs()->songs().sort(sorter, false);
}
