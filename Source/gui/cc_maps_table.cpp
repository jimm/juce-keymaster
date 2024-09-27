#include "../km/consts.h"
#include "../km/controller.h"
#include "../km/connection.h"
#include "cc_maps_table.h"
#include "cc_map_dialog_component.h"

void CcMapsTableListBoxModel::set_connection(Connection *c)
{
  _conn = c;
  for (int i = 0; i < 128; ++i) {
    auto *controller = _conn->cc_map(i);
    if (controller != nullptr)
      _cc_maps.add(controller);
  }
}

void CcMapsTableListBoxModel::make_columns(TableHeaderComponent &header) {
  header.addColumn("CC In", 1, 20);
  header.addColumn("CC Out", 2, 20);
  header.addColumn("Filtered", 3, 20);
  header.addColumn("Pass 0", 4, 20);
  header.addColumn("Pass 127", 5, 20);
  header.addColumn("Min In", 6, 20);
  header.addColumn("Max In", 7, 20);
  header.addColumn("Min Out", 8, 20);
  header.addColumn("Max Out", 9, 20);
  header.setStretchToFitActive(true);
}

void CcMapsTableListBoxModel::paintCell(
  Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
  Controller *c = _cc_maps[rowNumber];
  KmTableListBoxModel::paintCell(g, rowNumber, columnId, width, height, rowIsSelected);
  String str;

  switch (columnId) {
  case 1:                       // CC In
    str = String(c->cc_num());
    break;
  case 2:                       // CC Out
    str = String(c->translated_cc_num());
    break;
  case 3:                       // Filtered
    str = c->filtered() ? "yes" : "no";
    break;
  case 4:                       // Pass 0
    str = c->pass_through_0() ? "yes" : "no";
    break;
  case 5:                       // Pass 127
    str = c->pass_through_127() ? "yes" : "no";
    break;
  case 6:                       // Min In
    str = String(c->min_in());
    break;
  case 7:                       // Max In
    str = String(c->max_in());
    break;
  case 8:                       // Min Out
    str = String(c->min_out());
    break;
  case 9:                       // Max Out
    str = String(c->max_out());
    break;
  }

  g.drawText(str, 2, 0, width - 4, height, Justification::centredLeft, true);
}

void CcMapsTableListBoxModel::cellDoubleClicked(int row, int col, const MouseEvent&) {
  open_cc_map_editor(_conn, _cc_maps[row], _list_box);
}
