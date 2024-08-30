#include <stdlib.h>
#include <string.h>
#include "set_list.h"
#include "keymaster.h"

SetList::SetList(DBObjID id, const String &name)
  : DBObj(id), Nameable(name)
{
}

SetList::~SetList() {
}

void SetList::add_song(Song *song) {
  if (this == KeyMaster_instance()->all_songs()) {
    auto sorter = NameableSorter();
    _songs.addSorted(sorter, song);
  }
  else
    _songs.add(song);
  KeyMaster_instance()->changed();
}

void SetList::remove_song(Song *song) {
  _songs.removeFirstMatchingValue(song);
  KeyMaster_instance()->changed();
}

void SetList::set_songs(Array<Song *>&other_songs) {
  if (_songs != other_songs) {
    _songs = other_songs;
    KeyMaster_instance()->changed();
  }
}
