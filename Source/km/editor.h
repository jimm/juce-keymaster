#pragma once

#include <JuceHeader.h>
#include "keymaster.h"

class Editor {
public:
  Editor(KeyMaster *km = nullptr); // defaults to KeyMaster_instance()

  MessageBlock *create_message();
  Trigger *create_trigger();
  Song *create_song();
  Patch *create_patch();
  Connection *create_connection(Input::Ptr input, Output::Ptr output);
  SetList *create_set_list();

  void add_message(MessageBlock *message) const;
  void add_trigger(Trigger *trigger) const;
  void add_song(Song *song) const;
  void add_patch(Patch *patch) const;
  void add_patch(Patch *patch, Song *song) const;
  void add_connection(Connection *connection, Patch *patch) const;
  void add_set_list(SetList *set_list) const;

  bool ok_to_destroy_message(MessageBlock *message);
  bool ok_to_destroy_trigger(Trigger *trigger);
  bool ok_to_destroy_song(Song *song);
  bool ok_to_destroy_patch(Song *song, Patch *patch);
  bool ok_to_destroy_connection(Patch *patch, Connection *connection);
  bool ok_to_destroy_set_list(SetList *set_list);

  void remove_song_from_set_list(SetList *set_list, Song *song);

  void destroy_message(MessageBlock *message);
  void destroy_trigger(Trigger *trigger);
  void destroy_song(Song *song);
  void destroy_patch(Song *song, Patch *patch);
  void destroy_connection(Patch *patch, Connection *connection);
  void destroy_set_list(SetList *set_list);

  void move_away_from_song(Song *);
  void move_away_from_patch(Song *, Patch *);

private:
  KeyMaster *km;
  Cursor *cursor;
};
