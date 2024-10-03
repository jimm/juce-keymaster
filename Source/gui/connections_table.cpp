#include "../km/consts.h"
#include "../km/connection.h"
#include "../km/patch.h"
#include "../km/editor.h"
#include "connections_table.h"
#include "connection_dialog_component.h"

void ConnectionsTableListBoxModel::make_columns(TableHeaderComponent &header) {
  header.addColumn("Input", 1, 75);
  header.addColumn("Chan", 2, 20, 20);
  header.addColumn("Output", 3, 75);
  header.addColumn("Chan", 4, 25, 25);
  header.addColumn("Zone", 5, 30);
  header.addColumn("Xpose", 6, 18, 18);
  header.addColumn("Prog", 7, 50);
  header.addColumn("Curve", 8, 20, 20);
  header.addColumn("CC Filts/Maps", 9, 200);
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
      str = c->input()->info.name;
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
      str = c->output()->info.name;
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
  Patch *p = KeyMaster_instance()->cursor()->patch();
  Connection *c = p->connections()[row];
  open_connection_editor(p, c)->addActionListener(this);
}

void ConnectionsTableListBox::popupMenu() {
  PopupMenu menu;
  Patch *patch = KeyMaster_instance()->cursor()->patch();

  menu.addItem("Create New Connection", [this, patch] {
    Editor e;
    Connection *conn = e.create_connection(nullptr, nullptr);
    e.add_connection(patch, conn);
    updateContent();
    repaint();
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
