#include "set_list_songs_list_box.h"
#include "song_editor.h"
#include "../km/keymaster.h"
#include "../km/editor.h"

void SetListSongsListBoxModel::listBoxItemDoubleClicked(int row, const MouseEvent&) {
  sendActionMessage("open:song-editor");
}

void SetListSongsListBox::popupMenu() {
  KeyMaster *km = KeyMaster_instance();

  if (static_cast<SetListSongsListBoxModel *>(getListBoxModel())->set_list() == km->all_songs())
    popup_all_songs_menu();
  else
    popup_set_list_menu();
}

void SetListSongsListBox::popup_all_songs_menu() {
  PopupMenu menu;
  KeyMaster *km = KeyMaster_instance();
  SetList *set_list = km->all_songs();

  menu.addItem("Create New Song", [this] {
    open_song_editor(nullptr)->addActionListener(this);
  });

  auto rows = getSelectedRows();
  if (rows.size() > 0) {
    menu.addItem("Delete Selected Song", [&] {
      Editor e;
      Song *song = set_list->songs()[rows[0]];

      e.remove_song_from_set_list(set_list, song); // destroys the song as well
      sendActionMessage("update:all");
    });
  }

  menu.showMenuAsync(PopupMenu::Options{}.withMousePosition());
}

void SetListSongsListBox::popup_set_list_menu() {
  PopupMenu menu;
  SetList *set_list = static_cast<SetListSongsListBoxModel *>(getListBoxModel())->set_list();

  menu.addItem("Add Song to Song List", [&] {
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

  auto rows = getSelectedRows();
  if (rows.size() > 0) {
    menu.addItem("Remove Selected Song from Song List", [&] {
      Editor e;

      e.remove_song_from_set_list(set_list, set_list->songs()[rows[0]]);
      sendActionMessage("update:all");
    });
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
