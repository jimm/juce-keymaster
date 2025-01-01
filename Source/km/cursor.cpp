#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include "consts.h"
#include "cursor.h"
#include "keymaster.h"

/*
 * A Cursor knows the current SetList, Song, and Patch, how to move between
 * songs and patches, and how to find them given name regexes.
 */

static const String moved = "moved";

Cursor::Cursor(KeyMaster *key_master)
  : km(key_master)
{
  clear();
}

Cursor::~Cursor() {
}

void Cursor::clear() {
  set_list_index = song_index = patch_index = UNDEFINED;
  connection_index = message_index = trigger_index = UNDEFINED;
}

/*
 * Set @set_list to All Songs, @song to first song, and
 * @patch to song's first patch. Song and patch may be +nil+.
 */
void Cursor::init() {
  set_list_index = 0;

  SetList *sl = set_list();
  if (sl != nullptr && !sl->songs().isEmpty()) {
    song_index = 0;
    Song *s = song();
    patch_index = (s == nullptr || s->patches().isEmpty()) ? UNDEFINED : 0;
  }
  else {
    song_index = UNDEFINED;
    patch_index = UNDEFINED;
  }
  sendActionMessage(moved);
}

SetList *Cursor::set_list() const {
  if (set_list_index == UNDEFINED)
    return km->all_songs();
  return km->set_lists()[set_list_index];
}

Song *Cursor::song() const {
  SetList *sl = set_list();
  if (sl == nullptr || song_index == UNDEFINED || sl->songs().isEmpty())
    return nullptr;
  return sl->songs()[song_index];
}

Patch *Cursor::patch() const {
  Song *s = song();
  if (s == nullptr || patch_index == UNDEFINED)
    return nullptr;
  return s->patches()[patch_index];
}

Connection *Cursor::connection() const {
  Patch *p = patch();
  if (p == nullptr || connection_index == UNDEFINED)
    return nullptr;
  return p->connections()[connection_index];
}

MessageBlock *Cursor::message() const {
  if (message_index == UNDEFINED)
    return nullptr;
  return km->messages()[message_index];
}

Trigger *Cursor::trigger() const {
  if (trigger_index == UNDEFINED)
    return nullptr;
  return km->triggers()[trigger_index];
}

Song * Cursor::next_song() {
  if (set_list_index == UNDEFINED)
    return nullptr;

  SetList *sl = set_list();
  if (song_index == sl->songs().size()-1)
    return nullptr;

  ++song_index;
  patch_index = 0;
  sendActionMessage(moved);

  return song();
}

Song * Cursor::prev_song() {
  if (set_list_index == UNDEFINED || song_index == 0)
    return nullptr;

  --song_index;
  patch_index = 0;
  sendActionMessage(moved);

  return song();
}

Patch * Cursor::next_patch() {
  Song *s = song();
  if (s == nullptr)
    return nullptr;

  if (patch_index == s->patches().size()-1) {
    next_song();
    return patch();
  }

  ++patch_index;
  sendActionMessage(moved);

  return patch();
}

Patch * Cursor::prev_patch() {
  if (song() == nullptr)
    return nullptr;

  if (patch_index == 0) {
    prev_song();
    return patch();
  }

  --patch_index;
  sendActionMessage(moved);

  return patch();
}

bool Cursor::has_next_song() {
  return set_list_index != UNDEFINED && song_index != set_list()->songs().size() - 1;
}

bool Cursor::has_prev_song() {
  return set_list_index != UNDEFINED && song_index != 0;
}

bool Cursor::has_next_patch() {
  Song *s = song();
  return s != nullptr && (patch_index != s->patches().size() - 1 || has_next_song());
}

bool Cursor::has_prev_patch() {
  Song *s = song();
  return s != nullptr && (patch_index > 0 || has_prev_song());
}

bool Cursor::has_next_patch_in_song() {
  Song *s = song();
  return s != nullptr && patch_index != s->patches().size() - 1;
}

bool Cursor::has_prev_patch_in_song() {
  Song *s = song();
  return s != nullptr && patch_index > 0;
}

void Cursor::jump_to_set_list_index(int i) {
  if (i == set_list_index || i < 0 || i >= km->set_lists().size())
    return;

  auto old_patch = patch();
  if (old_patch)
    old_patch->stop();

  set_list_index = i;
  auto slist = set_list();

  song_index = slist->songs().isEmpty() ? UNDEFINED : 0;
  auto s = song();
  if (s == nullptr) {
    song_index = patch_index = connection_index = UNDEFINED;
    sendActionMessage(moved);
    return;
  }

  patch_index = s->patches().isEmpty() ? UNDEFINED : 0;
  if (patch_index == UNDEFINED) {
    connection_index = UNDEFINED;
    sendActionMessage(moved);
    return;
  }

  auto p = patch();
  connection_index = p->connections().isEmpty() ? UNDEFINED : 0;
  p->start();
  sendActionMessage(moved);
}

void Cursor::jump_to_song_index(int i) {
  if (i == song_index)
    return;

  auto slist = set_list();
  if (slist == nullptr || i < 0 || i >= slist->songs().size())
    return;

  auto old_patch = patch();
  if (old_patch)
    old_patch->stop();

  song_index = i;
  auto s = song();
  if (s == nullptr) {
    song_index = patch_index = connection_index = UNDEFINED;
    sendActionMessage(moved);
    return;
  }

  patch_index = s->patches().isEmpty() ? UNDEFINED : 0;
  if (patch_index == UNDEFINED) {
    connection_index = UNDEFINED;
    sendActionMessage(moved);
    return;
  }

  auto p = patch();
  connection_index = p->connections().isEmpty() ? UNDEFINED : 0;
  p->start();
  sendActionMessage(moved);
}

void Cursor::jump_to_patch_index(int i) {
  if (i == patch_index)
    return;

  auto s = song();
  if (s == nullptr || i < 0 || i >= s->patches().size())
    return;

  auto old_patch = patch();
  if (old_patch != nullptr)
    old_patch->stop();

  patch_index = i;
  auto p = patch();
  connection_index = p->connections().isEmpty() ? UNDEFINED : 0;
  p->start();
  sendActionMessage(moved);
}

void Cursor::jump_to_connection_index(int i) {
  if (i == connection_index)
    return;

  auto p = patch();
  if (p == nullptr || i < 0 || i >= p->connections().size())
    return;

  auto old_conn = connection();
  if (old_conn != nullptr)
    old_conn->stop();

  connection_index = i;
  auto conn = connection();
  if (conn != nullptr)
    conn->start();
  sendActionMessage(moved);
}

void Cursor::jump_to_message_index(int i) {
  if (i != message_index) {
    message_index = i;
    sendActionMessage(moved);
  }
}

void Cursor::jump_to_trigger_index(int i) {
  if (i != trigger_index) {
    trigger_index = i;
    sendActionMessage(moved);
  }
}

// If `song` is in current set list, make it the current song. If not, don't
// do anything.
void Cursor::goto_song(Song *s) {
  if (s == song())
    return;

  SetList *sl = set_list();
  int i = 0;
  for (auto &sl_song : sl->songs()) {
    if (sl_song == s) {
      song_index = i;
      patch_index = 0;
      sendActionMessage(moved);
      return;
    }
    ++i;
  }
}

// If `patch` is in current song, make it the current patch. If not, don't
// do anything.
void Cursor::goto_patch(Patch *p) {
  Song *curr_song = song();
  if (curr_song == nullptr || p == patch())
    return;

  int i = 0;
  for (auto &s_patch : curr_song->patches()) {
    if (s_patch == p) {
      patch_index = i;
      sendActionMessage(moved);
      return;
    }
    ++i;
  }
}

void Cursor::goto_song(const String &name_regex) {
  SetList *sl = set_list();
  regex_t re;
  regmatch_t match;
  int i;

  if (regcomp(&re, name_regex.getCharPointer(), REG_EXTENDED | REG_ICASE) != 0)
    return;

  if (sl != nullptr) {
    i = 0;
    for (auto &song : sl->songs()) {
      if (regexec(&re, song->name().getCharPointer(), 1, &match, 0) == 0) {
        song_index = i;
        patch_index = 0;
        sendActionMessage(moved);
        return;
      }
      ++i;
    }
  }

  i = 0;
  for (auto &song : km->all_songs()->songs()) {
    if (regexec(&re, song->name().getCharPointer(), 1, &match, 0) == 0) {
      set_list_index = 0;
      song_index = i;
      patch_index = 0;
        sendActionMessage(moved);
      return;
    }
    ++i;
  }
}

void Cursor::goto_set_list(const String &name_regex) {
  regex_t re;
  regmatch_t match;

  if (regcomp(&re, name_regex.getCharPointer(), REG_EXTENDED | REG_ICASE) != 0)
    return;

  int i = 0;
  for (auto &set_list : km->set_lists()) {
    if (regexec(&re, set_list->name().getCharPointer(), 1, &match, 0) == 0) {
      set_list_index = i;
      song_index = 0;
      patch_index = 0;
      sendActionMessage(moved);
      return;
    }
    ++i;
  }
}
