#include "song_patches_list_box.h"
#include "patch_editor.h"
#include "../km/keymaster.h"
#include "../km/editor.h"

void SongPatchesListBoxModel::listBoxItemDoubleClicked(int row, const MouseEvent&) {
  sendActionMessage("open:patch-editor");
}

void SongPatchesListBox::popupMenu() {
  PopupMenu menu;

  menu.addItem("Create New Patch", [this] {
    open_patch_editor(nullptr)->addActionListener(this);
  });

  auto rows = getSelectedRows();
  if (rows.size() > 0) {
    menu.addItem("Delete Selected Patch", [&] {
      Editor e;
      auto song = KeyMaster_instance()->cursor()->song();
      auto patch = song->patches()[rows[0]];

      e.destroy_patch(song, patch);
      sendActionMessage("update:all");
    });
  }

  menu.showMenuAsync(PopupMenu::Options{}.withMousePosition());
}

void SongPatchesListBox::actionListenerCallback(const String &message) {
  if (message == "open:patch-editor") {
    auto patch = KeyMaster_instance()->cursor()->patch();
    open_patch_editor(patch)->addActionListener(this);
  }
  else KmListBox::actionListenerCallback(message);
}
