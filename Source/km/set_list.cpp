#include <stdlib.h>
#include <string.h>
#include "set_list.h"

SetList::SetList(DBObjID id, const String &name)
  : DBObj(id), Nameable(name)
{
}

SetList::~SetList() {
}

void SetList::add_song(Song *song) {
  _songs.add(song);
}

void SetList::remove_song(Song *song) {
  _songs.removeFirstMatchingValue(song);
}
