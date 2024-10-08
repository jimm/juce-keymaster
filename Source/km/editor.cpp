#include "consts.h"
#include "editor.h"
#include "keymaster.h"
#include "cursor.h"

Editor::Editor(KeyMaster *key_master)
  : km(key_master ? key_master : KeyMaster_instance())
{
  cursor = km->cursor();
}

MessageBlock *Editor::create_message() const {
  return new MessageBlock(UNDEFINED_ID, "New Message");
}

Trigger *Editor::create_trigger() const {
  return new Trigger(UNDEFINED_ID, "New Trigger", TriggerAction::NEXT_PATCH, nullptr);
}

Song *Editor::create_song() const {
  return new Song(UNDEFINED_ID, "New Song");
}

Patch *Editor::create_patch() const {
  return new Patch(UNDEFINED_ID, "New Patch");
}

Connection *Editor::create_connection(Input::Ptr input, Output::Ptr output) const
{
  return new Connection(UNDEFINED_ID, input, CONNECTION_ALL_CHANNELS, output, CONNECTION_ALL_CHANNELS);
}

SetList *Editor::create_set_list() const {
  return new SetList(UNDEFINED_ID, "New Set List");
}

void Editor::add_message(MessageBlock *message) const {
  km->add_message(message);
  cursor->message_index = km->messages().size() - 1;
}

void Editor::add_trigger(Trigger *trigger) const {
  km->add_trigger(trigger);
  cursor->trigger_index = km->messages().size() - 1;
}

void Editor::add_song(Song *song) const {
  km->all_songs()->add_song(song);

  SetList *curr_set_list = cursor->set_list();
  if (curr_set_list == nullptr)
    curr_set_list = km->all_songs();
  if (curr_set_list == km->all_songs()) {
    km->goto_song(song);
    return;
  }

  Array<Song *> &slist = curr_set_list->songs();
  Song *curr_song = cursor->song();
  if (curr_song == nullptr)
    slist.add(song);
  else {
    int pos = slist.indexOf(curr_song);
    slist.insert(pos + 1, song);
  }

  km->goto_song(song);
}

void Editor::add_patch(Patch *patch) const {
  add_patch(patch, cursor->song());
}

void Editor::add_patch(Patch *patch, Song *song) const {
  if (song == nullptr) {
    song = create_song();
    add_song(song);
  }
  song->patches().add(patch);
  km->goto_patch(patch);
}

void Editor::add_connection(Patch *patch, Connection *connection) const {
  if (patch == nullptr) {
    patch = create_patch();
    add_patch(patch, cursor->song());
  }

  patch->add_connection(connection);
  cursor->connection_index = patch->connections().size() - 1;
}

void Editor::add_set_list(SetList *set_list) const {
  km->add_set_list(set_list);
  cursor->set_list_index = km->set_lists().size() - 1;
  cursor->init();
}

void Editor::remove_song_from_set_list(SetList *set_list, Song *song) const {
  // If all songs, destroy the song completely
  if (set_list == km->all_songs()) {
    destroy_song(song);
    return;
  }

  if (song == cursor->song())
    move_away_from_song(song);
  set_list->remove_song(song);
}


void Editor::destroy_message(MessageBlock *message) const {
  int index = km->messages().indexOf(message);
  km->remove_message(message);
  if (index >= km->messages().size())
    cursor->message_index = km->messages().size() - 1;
}

void Editor::destroy_trigger(Trigger *trigger) const {
  int index = km->triggers().indexOf(trigger);
  km->remove_trigger(trigger);
  if (index >= km->triggers().size())
    cursor->message_index = km->triggers().size() - 1;
}

// Returns true if `message` is not used anywhere.
bool Editor::ok_to_destroy_message(MessageBlock *message) const {
  if (message == nullptr)
    return false;

  for (auto &song : km->all_songs()->songs())
    for (auto &patch : song->patches())
      if (patch->start_message() == message || patch->stop_message() == message)
        return false;

  for (auto &trigger : km->triggers())
    if (trigger->output_message() == message)
      return false;

  return true;
}

bool Editor::ok_to_destroy_set_list(SetList *set_list) const {
  return set_list != nullptr
    && set_list != km->all_songs();
}

// Destroys the song and removes it from all set lists. If it is the current
// song, calls move_away_from_song() first.
void Editor::destroy_song(Song *song) const {
  move_away_from_song(song);
  for (auto &set_list : km->set_lists())
    set_list->remove_song(song);
  km->all_songs()->remove_song(song);
  delete song;

  if (cursor->patch())
    cursor->patch()->start();
}

void Editor::destroy_patch(Song *song, Patch *patch) const {
  if (cursor->patch())
    cursor->patch()->stop();

  // Tell all inputs to deal with hanging notes and sustains now
  for (auto inp : KeyMaster_instance()->device_manager().inputs())
    inp->patch_being_deleted(patch);

  move_away_from_patch(song, patch);
  song->remove_patch(patch);

  if (cursor->patch())
    cursor->patch()->start();
}

void Editor::destroy_connection(Patch *patch, Connection *connection) const {
  int index = cursor->connection_index;
  patch->remove_connection(connection);
  if (index >= patch->connections().size())
    cursor->connection_index = patch->connections().size() - 1;
}

void Editor::destroy_set_list(SetList *set_list) const {
  if (set_list == cursor->set_list())
    cursor->goto_set_list("All Songs");
  km->remove_set_list(set_list);
}

// If `song` is not the current song, does nothing. Else tries to move to
// the next song (see comment though) or, if there isn't one, the prev song.
// If both those fail (this is the only song in the current set list) then
// reinits the cursor.
void Editor::move_away_from_song(Song *song) const {
  if (song != cursor->song())
    return;

  if (cursor->has_next_song()) {
    // Don't move to next song because this one will be deleted and the
    // cursor index will point to the next song. Do set the cursor's patch
    // index back to 0.
    cursor->patch_index = 0;
    return;
  }

  if (cursor->has_prev_song()) {
    km->prev_song();
    return;
  }

  // Nowhere to move. Reinit the cursor.
  cursor->init();
}

// If `patch` is not the current patch, does nothing. Else tries to move to
// the next patch or, if there isn't one, the prev patch. If both those fail
// (this is the only patch in the current song) then calls
// move_away_from_song.
void Editor::move_away_from_patch(Song *song, Patch *patch) const {
  if (patch != cursor->patch())
    return;

  if (cursor->has_next_patch_in_song()) {
    // Do nothing. We'll remove the patch from the song. The cursor patch
    // index will remain the same, so it will point to the patch that is
    // currently after this one.
    return;
  }

  if (cursor->has_prev_patch_in_song()) {
    km->prev_patch();
    return;
  }

  move_away_from_song(song);
}
