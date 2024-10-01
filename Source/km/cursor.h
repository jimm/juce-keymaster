#pragma once

#include "set_list.h"
#include "song.h"
#include "patch.h"
#include "connection.h"
#include "message_block.h"
#include "trigger.h"

class KeyMaster;

class Cursor : public ActionBroadcaster {
public:
  KeyMaster *km;
  int set_list_index;
  int song_index;
  int patch_index;

  // These are needed for editing though not for MIDI processing
  int connection_index;
  int message_index;
  int trigger_index;

  Cursor(KeyMaster *km);
  ~Cursor();

  void clear();
  void init();

  SetList *set_list() const;
  Song *song() const;
  Patch *patch() const;

  // For editing
  Connection *connection() const;
  MessageBlock *message() const;
  Trigger *trigger() const;

  Song * next_song(bool send_changed = true);
  Song * prev_song(bool send_changed = true);
  Patch * next_patch(bool send_changed = true);
  Patch * prev_patch(bool send_changed = true);

  bool has_next_song();
  bool has_prev_song();
  bool has_next_patch();
  bool has_prev_patch();

  bool has_next_patch_in_song();
  bool has_prev_patch_in_song();

  void jump_to_set_list_index(int i);
  void jump_to_song_index(int i);
  void jump_to_patch_index(int i);
  void jump_to_connection_index(int i);
  void jump_to_message_index(int i);
  void jump_to_trigger_index(int i);

  void goto_song(Song *song);
  void goto_patch(Patch *patch);
  void goto_connection(Connection *connection);
  void goto_message(MessageBlock *trigger);
  void goto_trigger(Trigger *trigger);

  void goto_song(const String &name_regex);
  void goto_set_list(const String &name_regex);
};
