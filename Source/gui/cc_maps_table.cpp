#include "../km/consts.h"
#include "../km/controller.h"
#include "../km/connection.h"
#include "cc_maps_table.h"
#include "cc_map_editor.h"

Controller * CcMapsTableListBoxModel::nth_cc_map(int n) {
  int count = 0;
  for (int i = 0; i < 128; ++i) {
    auto controller = _conn->cc_map(i);
    if (controller != nullptr) {
      if (count++ == n)
        return controller;
    }
  }
  return nullptr;
}

int CcMapsTableListBoxModel::getNumRows() {
  int count = 0;
  for (int i = 0; i < 128; ++i)
    if (_conn->cc_map(i) != nullptr)
      ++count;
  return count;
}

void CcMapsTableListBoxModel::make_columns(TableHeaderComponent &header) {
  auto props = TableHeaderComponent::ColumnPropertyFlags::notSortable;
  header.addColumn("CC In", 1, 20, 20, -1, props);
  header.addColumn("CC Out", 2, 20, 20, -1, props);
  header.addColumn("Filtered", 3, 20, 20, -1, props);
  header.addColumn("Pass 0", 4, 20, 20, -1, props);
  header.addColumn("Pass 127", 5, 20, 20, -1, props);
  header.addColumn("Min In", 6, 20, 20, -1, props);
  header.addColumn("Max In", 7, 20, 20, -1, props);
  header.addColumn("Min Out", 8, 20, 20, -1, props);
  header.addColumn("Max Out", 9, 20, 20, -1, props);
  header.setStretchToFitActive(true);
}

void CcMapsTableListBoxModel::paintCell(
  Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
  Controller *c = nth_cc_map(rowNumber);
  bool filtered = c->filtered();
  KmTableListBoxModel::paintCell(g, rowNumber, columnId, width, height, rowIsSelected);
  String str;

  switch (columnId) {
  case 1:                       // CC In
    str = String(c->cc_num());
    break;
  case 2:                       // CC Out
    str = filtered ? "" : String(c->translated_cc_num());
    break;
  case 3:                       // Filtered
    str = filtered ? "yes" : "no";
    break;
  case 4:                       // Pass 0
    str = filtered ? "" : (c->pass_through_0() ? "yes" : "no");
    break;
  case 5:                       // Pass 127
    str = filtered ? "" : (c->pass_through_127() ? "yes" : "no");
    break;
  case 6:                       // Min In
    str = filtered ? "" : String(c->min_in());
    break;
  case 7:                       // Max In
    str = filtered ? "" : String(c->max_in());
    break;
  case 8:                       // Min Out
    str = filtered ? "" : String(c->min_out());
    break;
  case 9:                       // Max Out
    str = filtered ? "" : String(c->max_out());
    break;
  }

  g.drawText(str, 2, 0, width - 4, height, Justification::centredLeft, true);
}

void CcMapsTableListBoxModel::cellDoubleClicked(int row, int col, const MouseEvent&) {
  _double_clicked_controller = nth_cc_map(row);
  sendActionMessage("open:cc-map-editor");
}

void CcMapsTableListBox::actionListenerCallback(const String &message) {
  if (message == "open:cc-map-editor") {
    auto model = static_cast<CcMapsTableListBoxModel *>(getTableListBoxModel());
    open_cc_map_editor(model->connection(), model->double_clicked_controller())->addActionListener(this);
  }
  else
    KmTableListBox::actionListenerCallback(message);
}
