#include <stdlib.h>
#include <string.h>
#include "song.h"

Song::Song(DBObjID id, const String &name)
  : DBObj(id), Nameable(name), _bpm(120.0), _clock_on_at_start(false)
{
}

Song::~Song() {
  for (auto& patch : _patches)
    delete patch;
}

void Song::add_patch(Patch *patch) {
  _patches.add(patch);
}

void Song::remove_patch(Patch *patch) {
  _patches.removeFirstMatchingValue(patch);
  delete patch;
}
