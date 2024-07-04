#pragma once

#include <JuceHeader.h>
#include "keymaster.h"
#include "midi_device.h"

class Editor {
public:
  Editor(KeyMaster *km = nullptr); // defaults to KeyMaster_instance()

  MessageBlock *create_message();
  Trigger *create_trigger(MidiInput *input);
  Song *create_song();
  Patch *create_patch();
  Connection *create_connection(MidiInputEntry::Ptr input, MidiOutputEntry::Ptr output);
  SetList *create_set_list();

  void add_message(MessageBlock *message);
  void add_trigger(Trigger *trigger);
  void add_song(Song *song);
  void add_patch(Patch *patch);
  void add_patch(Patch *patch, Song *song);
  void add_connection(Connection *connection, Patch *patch);
  void add_set_list(SetList *set_list);

  bool ok_to_destroy_message(MessageBlock *message);
  bool ok_to_destroy_trigger(Trigger *trigger);
  bool ok_to_destroy_song(Song *song);
  bool ok_to_destroy_patch(Song *song, Patch *patch);
  bool ok_to_destroy_connection(Patch *patch, Connection *connection);
  bool ok_to_destroy_set_list(SetList *set_list);

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
};
