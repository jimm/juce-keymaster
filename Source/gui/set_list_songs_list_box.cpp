#include "set_list_songs_list_box.h"
#include "song_editor.h"
#include "../km/keymaster.h"
#include "../km/editor.h"

void SetListSongsListBoxModel::listBoxItemDoubleClicked(int row, const MouseEvent&) {
  sendActionMessage("open:song-editor");
}

void SetListSongsListBox::popupMenu() {
  PopupMenu menu;
  KeyMaster *km = KeyMaster_instance();
  SetList *set_list = static_cast<SetListSongsListBoxModel *>(getListBoxModel())->set_list();
  bool is_all_songs = set_list == km->all_songs();
  auto rows = getSelectedRows();
  int selected_index = rows.size() > 0 ? rows[0] : UNDEFINED;
  Song *song = selected_index == UNDEFINED ? nullptr : set_list->songs()[selected_index];

  if (song) {
    menu.addItem("Edit Song", [this, song] {
      open_song_editor(song)->addActionListener(this);
    });
  }

  menu.addItem("Create New Song", [this] {
    open_song_editor(nullptr)->addActionListener(this);
  });

  if (is_all_songs) {
    if (song) {
      menu.addItem("Delete Selected Song", [this, set_list, song] {
        Editor e;
        e.remove_song_from_set_list(set_list, song); // destroys the song as well
        sendActionMessage("update:all");
      });
    }
  }
  else {
    PopupMenu submenu;
    for (auto *s : km->all_songs()->songs()) {
      submenu.addItem(s->name(), [this, km, set_list, selected_index, s] {
        set_list->insert_song(selected_index + 1, s);
        km->cursor()->goto_song(s);
        sendActionMessage("update:all");
      });
    }
    menu.addSubMenu("Add Song to Set List", submenu);

    if (song) {
      menu.addItem("Remove Selected Song from Set List", [this, set_list, song] {
        Editor e;
        e.remove_song_from_set_list(set_list, song);
        sendActionMessage("update:all");
      });
    }
  }

  menu.showMenuAsync(PopupMenu::Options{}.withMousePosition());
}

void SetListSongsListBox::actionListenerCallback(const String &message) {
  if (message == "open:song-editor") {
    Song *s = KeyMaster_instance()->cursor()->song();
    open_song_editor(s)->addActionListener(this);
  }
  else
    KmListBox::actionListenerCallback(message);
}
