#include <stdlib.h>
#include <string.h>
#include "song.h"
#include "keymaster.h"

Song::Song(DBObjID id, const String &name)
  : DBObj(id), Nameable(name), _bpm(120.0), _clock_on_at_start(false)
{
}

Song::~Song() {
  for (auto& patch : _patches)
    delete patch;
}

void Song::start() {
  Clock &clock = KeyMaster_instance()->clock();
  clock.set_bpm(_bpm);
  if (_clock_on_at_start)
    clock.start();
}

void Song::stop() {
  KeyMaster_instance()->clock().stop();
}

void Song::set_notes(const String &notes) {
  if (_notes != notes) {
    _notes = notes;
    KeyMaster_instance()->changed();
  }
}

void Song::set_bpm(float bpm) {
  if (std::abs(_bpm - bpm) > 0.001) {
    _bpm = bpm;
    KeyMaster_instance()->changed();
  }
}

void Song::set_clock_on_at_start(bool val) {
  if (_clock_on_at_start != val) {
    _clock_on_at_start = val;
    KeyMaster_instance()->changed();
  }
}

void Song::add_patch(Patch *patch) {
  _patches.add(patch);
  KeyMaster_instance()->changed();
}

void Song::remove_patch(Patch *patch) {
  _patches.removeFirstMatchingValue(patch);
  delete patch;
  KeyMaster_instance()->changed();
}
