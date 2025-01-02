#pragma once

#include "db_obj.h"
#include "song.h"

class SetList : public DBObj, public Nameable {
public:
  SetList(DBObjID id, const String &name);
  ~SetList();

  inline Array<Song *> &songs() { return _songs; }

  void add_song(Song *song);
  void insert_song(int index, Song *song);
  void remove_song(Song *song);

  void set_songs(Array<Song *>&other_songs);

protected:
  Array<Song *> _songs;
};
