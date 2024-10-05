#include "messages_list_box.h"
#include "message_editor.h"
#include "../km/editor.h"

void MessagesListBoxModel::listBoxItemDoubleClicked(int row, const MouseEvent&) {
  sendActionMessage("open:message-editor");
}

void MessagesListBox::popupMenu() {
  PopupMenu menu;
  auto messages = KeyMaster_instance()->messages();

  menu.addItem("Create New Message", [this] {
    open_message_editor(nullptr)->addActionListener(this);
  });

  auto rows = getSelectedRows();
  if (rows.size() > 0) {
    auto message = messages[rows[0]];
    menu.addItem("Delete Selected Message", [this, message] {
      Editor e;
      e.destroy_message(message);
      updateContent();
      repaint();
    });
  }

  menu.showMenuAsync(PopupMenu::Options{}.withMousePosition());
}

void MessagesListBox::actionListenerCallback(const String &message) {
  if (message == "open:message-editor") {
    auto model = static_cast<MessagesListBoxModel *>(getListBoxModel());
    auto m = KeyMaster_instance()->messages()[model->selected_row_num()];
    open_message_editor(m)->addActionListener(this);
  }
  else
    KmListBox::actionListenerCallback(message);
}
