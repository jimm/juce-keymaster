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
  Song *song = rows.size() > 0 ? set_list->songs()[rows[0]] : nullptr;

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
      menu.addItem("Delete Selected Song", [&] {
        Editor e;
        e.remove_song_from_set_list(set_list, song); // destroys the song as well
        sendActionMessage("update:all");
      });
    }
  }
  else {
    menu.addItem("Add Song to Set List", [&] {
      PopupMenu songsMenu;
      KeyMaster *km = KeyMaster_instance();

      for (auto *song : km->all_songs()->songs()) {
        menu.addItem(song->name(), [&] {
          Editor e;
          set_list->add_song(song);
          km->cursor()->goto_song(song);
          sendActionMessage("update:all");
        });
      }
    });

    if (song) {
      menu.addItem("Remove Selected Song from Set List", [&] {
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
