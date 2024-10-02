#include "set_lists_list_box.h"
#include "../km/keymaster.h"
#include "../km/editor.h"

void SetListsListBox::popupMenu() {
  PopupMenu menu;
  KeyMaster *km = KeyMaster_instance();

  menu.addItem("Create New Set List", [this] {
    Editor e;
    SetList *set_list = e.create_set_list();
    e.add_set_list(set_list);
    sendActionMessage("update:all");
  });

  auto rows = getSelectedRows();
  if (rows[0] != 0) {           // can't delete list of all songs
    menu.addItem("Delete Selected Set List", [&] {
      Editor e;
      auto set_list = km->set_lists()[rows[0]];

      e.destroy_set_list(set_list);
      sendActionMessage("update:all");
    });
  }

  menu.showMenuAsync(PopupMenu::Options{}.withMousePosition());
}
