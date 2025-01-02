#include "song_patches_list_box.h"
#include "patch_editor.h"
#include "../km/keymaster.h"
#include "../km/editor.h"

void SongPatchesListBoxModel::listBoxItemDoubleClicked(int row, const MouseEvent&) {
  sendActionMessage("open:patch-editor");
}

void SongPatchesListBox::popupMenu() {
  PopupMenu menu;
  auto song = KeyMaster_instance()->cursor()->song();
  auto rows = getSelectedRows();
  Patch *patch = rows.size() > 0 ? song->patches()[rows[0]] : nullptr;

  if (patch) {
    menu.addItem("Edit Patch", [this, patch] {
      open_patch_editor(patch)->addActionListener(this);
    });
  }

  menu.addItem("Create New Patch", [this] {
    open_patch_editor(nullptr)->addActionListener(this);
  });

  if (patch) {
    menu.addItem("Delete Selected Patch", [&] {
      Editor e;
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
