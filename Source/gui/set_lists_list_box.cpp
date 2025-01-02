#include "set_lists_list_box.h"
#include "set_list_editor.h"
#include "../km/keymaster.h"
#include "../km/editor.h"

void SetListsListBoxModel::listBoxItemDoubleClicked(int row, const MouseEvent&) {
  sendActionMessage("open:set-list-editor");
}

void SetListsListBox::popupMenu() {
  PopupMenu menu;
  KeyMaster *km = KeyMaster_instance();
  auto rows = getSelectedRows();
  SetList *set_list = rows.size() > 0 ? km->set_lists()[rows[0]] : nullptr;

  // Can't edit All Songs set list
  if (set_list == km->all_songs())
    set_list = nullptr;

  if (set_list) {
    menu.addItem("Edit Set List", [this, set_list] {
      open_set_list_editor(set_list)->addActionListener(this);
    });
  }

  menu.addItem("Create New Set List", [this] {
    Editor e;
    SetList *set_list = e.create_set_list();
    e.add_set_list(set_list);
    sendActionMessage("update:all");
  });

  if (set_list) {
    menu.addItem("Delete Selected Set List", [&] {
      Editor e;
      e.destroy_set_list(set_list);
      sendActionMessage("update:all");
    });
  }

  menu.showMenuAsync(PopupMenu::Options{}.withMousePosition());
}

void SetListsListBox::actionListenerCallback(const String &message) {
  if (message == "open:set-list-editor") {
    KeyMaster *km = KeyMaster_instance();
    SetList *sl = km->cursor()->set_list();
    if (sl != km->all_songs())
      open_set_list_editor(sl)->addActionListener(this);
  }
  else
    KmListBox::actionListenerCallback(message);
}
