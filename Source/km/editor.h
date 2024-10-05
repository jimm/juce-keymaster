#pragma once

#include <JuceHeader.h>
#include "keymaster.h"

class Editor {
public:
  Editor(KeyMaster *km = nullptr); // defaults to KeyMaster_instance()

  MessageBlock *create_message() const;
  Trigger *create_trigger() const;
  Song *create_song() const;
  Patch *create_patch() const;
  Connection *create_connection(Input::Ptr input, Output::Ptr output) const;
  SetList *create_set_list() const;

  void add_message(MessageBlock *message) const;
  void add_trigger(Trigger *trigger) const;
  void add_song(Song *song) const;
  void add_patch(Patch *patch) const;
  void add_patch(Patch *patch, Song *song) const;
  void add_connection(Patch *patch, Connection *connection) const;
  void add_set_list(SetList *set_list) const;

  bool ok_to_destroy_message(MessageBlock *message) const;
  bool ok_to_destroy_set_list(SetList *set_list) const;

  void remove_song_from_set_list(SetList *set_list, Song *song) const;

  void destroy_message(MessageBlock *message) const;
  void destroy_trigger(Trigger *trigger) const;
  void destroy_song(Song *song) const;
  void destroy_patch(Song *song, Patch *patch) const;
  void destroy_connection(Patch *patch, Connection *connection) const;
  void destroy_set_list(SetList *set_list) const;

  void move_away_from_song(Song *) const;
  void move_away_from_patch(Song *, Patch *) const;

private:
  KeyMaster *km;
  Cursor *cursor;
};
