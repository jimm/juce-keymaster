#pragma once

#include <JuceHeader.h>
#include "km_table_list_box.h"
#include "../km/connection.h"

class ProgramChangesTableListBoxModel : public KmTableListBoxModel {
public:
  virtual ~ProgramChangesTableListBoxModel() {}

  void make_columns(TableHeaderComponent &header) override;

  virtual int getNumRows() override;

  void paintCell(Graphics& g, int rowNumber, int columnId,
                 int width, int height, bool rowIsSelected) override;

  virtual void cellDoubleClicked(int row, int col, const MouseEvent&) override;

  Connection *connection() { return _conn; }
  void set_connection(Connection *c) { _conn = c; }

  ProgramChange *double_clicked_pc() { return _double_clicked_pc; }

private:
  Connection *_conn = nullptr;
  ProgramChange *_double_clicked_pc = nullptr;
};


class ProgramChangesTableListBox : public KmTableListBox {
public:
  virtual void actionListenerCallback(const String &message) override;
};
