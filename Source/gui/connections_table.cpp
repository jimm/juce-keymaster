#include "../km/consts.h"
#include "../km/connection.h"
#include "../km/patch.h"
#include "../km/editor.h"
#include "connections_table.h"
#include "connection_editor.h"

void ConnectionsTableListBoxModel::make_columns(TableHeaderComponent &header) {
  auto props = TableHeaderComponent::ColumnPropertyFlags::notSortable;
  header.addColumn("Input", 1, 75, 75, -1, props);
  header.addColumn("Chan", 2, 20, 20, -1, props);
  header.addColumn("Output", 3, 75, 75, -1, props);
  header.addColumn("Chan", 4, 25, 25, -1, props);
  header.addColumn("Zone", 5, 30, 30, -1, props);
  header.addColumn("Xpose", 6, 18, 18, -1, props);
  header.addColumn("Prog", 7, 50, 50, -1, props);
  header.addColumn("Curve", 8, 20, 20, -1, props);
  header.addColumn("CC Filts/Maps", 9, 200, 200, 200, props);
  header.setStretchToFitActive(true);
}

void ConnectionsTableListBoxModel::paintCell(
  Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
  KmTableListBoxModel::paintCell(g, rowNumber, columnId, width, height, rowIsSelected);

  Patch *p = patch();
  if (p == nullptr)
    return;

  Connection *c = p->connections()[rowNumber];
  Curve *curve = c->velocity_curve();
  String str;
  switch (columnId) {
  case 1:                       // input name
    if (c->input())
      str = c->input()->name();
    else
      str = "<not connected>";
    break;
  case 2:                       // input chan
    if (c->input_chan() == CONNECTION_ALL_CHANNELS)
      str = String("All");
    else
      str = String(c->input_chan() + 1);
    break;
  case 3:                       // output name
    if (c->output())
      str = c->output()->name();
    else
      str = "<not connected>";
    break;
  case 4:                       // output chan
    if (c->output_chan() == CONNECTION_ALL_CHANNELS)
      str = String("Input");
    else
      str = String(c->output_chan() + 1);
    break;
  case 5:                       // zone
    str = MidiMessage::getMidiNoteName(c->zone_low(), true, true, 4);
    str << " - ";
    str << MidiMessage::getMidiNoteName(c->zone_high(), true, true, 4);
    break;
  case 6:                       // xpose
    str = String(c->xpose());
    break;
  case 7:                       // prog
    str = program_str(c);
    break;
  case 8:                       // velocity curve
    str = curve == nullptr ? "lin" : curve->short_name();
    break;
  case 9:                       // CC filters / maps
    str = controllers_str(c);
    break;
  }

  g.drawText(str, 2, 0, width - 4, height, Justification::centredLeft, true);
}

void ConnectionsTableListBoxModel::cellDoubleClicked(int row, int col, const MouseEvent&) {
  sendActionMessage("open:connection-editor");
}

void ConnectionsTableListBox::popupMenu() {
  PopupMenu menu;
  auto patch = KeyMaster_instance()->cursor()->patch();

  menu.addItem("Create New Connection", [this] {
    open_connection_editor(nullptr)->addActionListener(this);
  });

  auto rows = getSelectedRows();
  if (rows.size() > 0) {
    auto conn = patch->connections()[rows[0]];
    menu.addItem("Delete Selected Connection", [this, patch, conn] {
      Editor e;
      e.destroy_connection(patch, conn);
      updateContent();
      repaint();
    });
  }

  menu.showMenuAsync(PopupMenu::Options{}.withMousePosition());
}

void ConnectionsTableListBox::actionListenerCallback(const String &message) {
  if (message == "open:connection-editor") {
    auto model = static_cast<ConnectionsTableListBoxModel *>(getTableListBoxModel());
    Patch *p = KeyMaster_instance()->cursor()->patch();
    Connection *c = p->connections()[model->selected_row_num()];
    open_connection_editor(c)->addActionListener(this);
  }
  else
    KmTableListBox::actionListenerCallback(message);
}

// ================ helpers ================

String ConnectionsTableListBoxModel::program_str(Connection *c) {
  String str;
  bool has_msb = c->program_bank_msb() != UNDEFINED;
  bool has_lsb = c->program_bank_lsb() != UNDEFINED;
  bool has_prog = c->program_prog() != UNDEFINED;

  if (has_msb || has_lsb) {
    str << '[';
    if (has_msb)
      str << String::formatted("%d", c->program_bank_msb());
    if (has_msb && has_lsb)
      str << ',';
    if (has_lsb)
      str << String::formatted("%d", c->program_bank_lsb());
    str << ']';
    if (has_prog)
      str << ' ';
  }

  if (has_prog)
    str << String::formatted("%d", c->program_prog());
    
  if (!str.isEmpty() && !c->program_change_can_be_sent())
    str = '(' + str + ')';

  return str;
}

String ConnectionsTableListBoxModel::controllers_str(Connection *c) {
  StringArray arr;

  for (int i = 0; i < 128; ++i) {
    Controller *cc = c->cc_map(i);
    if (cc == nullptr)
      continue;

    String str;
    str += cc->cc_num();

    if (cc->filtered()) {
      str += 'x';
      continue;
    }

    if (cc->cc_num() != cc->translated_cc_num())
      str += String::formatted("->%d", cc->translated_cc_num());

    if (cc->min_in() != 0 || cc->max_in() != 127
        || cc->min_out() != 0 || cc->max_out() != 127)
    {
      str += ' ';
      if (cc->pass_through_0())
        str += 0;
      str += String::formatted(" [%d, %d] -> [%d, %d]",
                               cc->min_in(), cc->max_in(),
                               cc->min_out(), cc->max_out());
      if (cc->pass_through_127())
        str += 127;
    }

    arr.add(str);
  }
  return arr.joinIntoString(", ");
}
