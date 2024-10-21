#include "../km/consts.h"
#include "../km/device_manager.h"
#include "../km/keymaster.h"
#include "../km/trigger.h"
#include "../km/editor.h"
#include "../km/formatter.h"
#include "triggers_table.h"
#include "trigger_editor.h"

void TriggersTableListBoxModel::make_columns(TableHeaderComponent &header) {
  auto props = TableHeaderComponent::ColumnPropertyFlags::notSortable;
  header.addColumn("Key", 1, 15, 15, -1, props);
  header.addColumn("MIDI", 2, 100, 100, -1, props);
  header.addColumn("Action / Message", 3, 100, 100, -1, props);
  header.setStretchToFitActive(true);
}

void TriggersTableListBoxModel::paintCell(
  Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
  KmTableListBoxModel::paintCell(g, rowNumber, columnId, width, height, rowIsSelected);

  Trigger *t = KeyMaster_instance()->triggers()[rowNumber];
  String str;
  switch (columnId) {
  case 1:                       // key
    if (t->has_trigger_key_press())
      str << t->trigger_key_press().getTextDescriptionWithIcons();
    break;
  case 2:                       // trigger
    if (t->has_trigger_message())
      str = trigger_message_description(t->trigger_message());
    break;
  case 3:                       // action / message
    str = action_string(t);
    break;
  }

  g.drawText(str, 2, 0, width - 4, height, Justification::centredLeft, true);
  g.setColour(_lf.findColour(ListBox::backgroundColourId));
  g.fillRect(width - 1, 0, 1, height);
}

String TriggersTableListBoxModel::action_string(Trigger *t) {
    switch (t->action()) {
    case TriggerAction::NEXT_SONG:
      return "Next Song";
    case TriggerAction::PREV_SONG:
      return "Prev Song";
    case TriggerAction::NEXT_PATCH:
      return "Next Patch";
    case TriggerAction::PREV_PATCH:
      return "Prev Patch";
    case TriggerAction::PANIC:
      return "Panic";
    case TriggerAction::SUPER_PANIC:
      return "Super Panic";
    case TriggerAction::TOGGLE_CLOCK:
      return "Toggle Clock";
    case TriggerAction::MESSAGE:
      return t->output_message()->name();
    }
    String error_msg;
    error_msg << "error: trigger has illegal action " << t->action();
    Logger::writeToLog(error_msg);
    return "";
}

void TriggersTableListBoxModel::cellDoubleClicked(int row, int col, const MouseEvent&) {
  sendActionMessage("open:trigger-editor");
}

void TriggersTableListBox::popupMenu() {
  PopupMenu menu;

  menu.addItem("Create New Trigger", [this] {
    open_trigger_editor(nullptr)->addActionListener(this);
  });

  auto rows = getSelectedRows();
  if (rows.size() > 0) {
    auto trigger = KeyMaster_instance()->triggers()[rows[0]];
    menu.addItem("Delete Selected Trigger", [this, trigger] {
      Editor e;
      e.destroy_trigger(trigger);
      updateContent();
      repaint();
    });
  }

  menu.showMenuAsync(PopupMenu::Options{}.withMousePosition());
}

void TriggersTableListBox::actionListenerCallback(const String &message) {
  if (message == "open:trigger-editor") {
    auto model = static_cast<TriggersTableListBoxModel *>(getTableListBoxModel());
    auto trigger = KeyMaster_instance()->triggers()[model->selected_row_num()];
    open_trigger_editor(trigger)->addActionListener(this);
  }
  else
    KmTableListBox::actionListenerCallback(message);
}
