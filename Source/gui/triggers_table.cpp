#include "../km/consts.h"
#include "../km/device_manager.h"
#include "../km/keymaster.h"
#include "../km/trigger.h"
#include "triggers_table.h"

void TriggersTableListBoxModel::make_columns(TableHeaderComponent &header) {
  header.addColumn("Key", 1, 15);
  header.addColumn("Input", 2, 75);
  header.addColumn("Trigger", 3, 75);
  header.addColumn("Action / Message", 4, 75);
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
    if (t->trigger_key_code() != UNDEFINED)
      str << KeyPress(t->trigger_key_code(), ModifierKeys(), juce_wchar(t->trigger_key_code()))
        .getTextDescription();
    break;
  case 2:                       // input
    str = input_string(t);
    break;
  case 3:                       // trigger
    if (!t->trigger_message().isActiveSense())
      str = t->trigger_message().getDescription();
    break;
  case 4:                       // action / message
    str = action_string(t);
    break;
  }

  g.drawText(str, 2, 0, width - 4, height, Justification::centredLeft, true);
  g.setColour(_lf.findColour(ListBox::backgroundColourId));
  g.fillRect(width - 1, 0, 1, height);
}

String TriggersTableListBoxModel::input_string(Trigger *t) {
  Input::Ptr input = t->trigger_input();
  return input ? input->info.name : "<not connected>";
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
      return "All Notes Off";
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
