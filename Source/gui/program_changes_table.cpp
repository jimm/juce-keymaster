#include "../km/consts.h"
#include "../km/connection.h"
#include "program_changes_table.h"
#include "program_change_editor.h"

int ProgramChangesTableListBoxModel::getNumRows() {
  return _conn ? _conn->program_changes().size() : 0;
}

void ProgramChangesTableListBoxModel::make_columns(TableHeaderComponent &header) {
  auto props = TableHeaderComponent::ColumnPropertyFlags::notSortable;
  header.addColumn("Output", 1, 50, 50, -1, props);
  header.addColumn("Chan", 2, 30, 30, -1, props);
  header.addColumn("MSB", 3, 20, 20, -1, props);
  header.addColumn("LSB", 4, 20, 20, -1, props);
  header.addColumn("Prog", 5, 20, 20, -1, props);
  header.setStretchToFitActive(true);
}

void ProgramChangesTableListBoxModel::paintCell(
  Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
  if (!_conn || rowNumber >= _conn->program_changes().size())
    return;
  ProgramChange *pc = _conn->program_changes()[rowNumber];
  KmTableListBoxModel::paintCell(g, rowNumber, columnId, width, height, rowIsSelected);
  String str;

  switch (columnId) {
  case 1:
    str = pc->output ? pc->output->name() : "(none)";
    break;
  case 2:
    str = pc->chan == CONNECTION_ALL_CHANNELS ? "" : String(pc->chan + 1);
    break;
  case 3:
    str = pc->bank_msb == UNDEFINED ? "" : String(pc->bank_msb);
    break;
  case 4:
    str = pc->bank_lsb == UNDEFINED ? "" : String(pc->bank_lsb);
    break;
  case 5:
    str = pc->prog == UNDEFINED ? "" : String(pc->prog);
    break;
  }

  g.drawText(str, 2, 0, width - 4, height, Justification::centredLeft, true);
}

void ProgramChangesTableListBoxModel::cellDoubleClicked(int row, int col, const MouseEvent&) {
  if (!_conn || row >= _conn->program_changes().size())
    return;
  _double_clicked_pc = _conn->program_changes()[row];
  sendActionMessage("open:program-change-editor");
}

void ProgramChangesTableListBox::actionListenerCallback(const String &message) {
  if (message == "open:program-change-editor") {
    auto model = static_cast<ProgramChangesTableListBoxModel *>(getTableListBoxModel());
    open_program_change_editor(model->connection(), model->double_clicked_pc())->addActionListener(this);
  }
  else
    KmTableListBox::actionListenerCallback(message);
}
