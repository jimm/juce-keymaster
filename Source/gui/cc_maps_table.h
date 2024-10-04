#pragma once

#include <JuceHeader.h>
#include "km_table_list_box.h"
#include "../km/keymaster.h"

class Connection;
class Controller;


class CcMapsTableListBoxModel : public KmTableListBoxModel {
public:
  virtual ~CcMapsTableListBoxModel() {}

  void make_columns(TableHeaderComponent &header) override;

  Controller *nth_cc_map(int n);

  virtual int getNumRows() override;

  void paintCell(Graphics& g, int rowNumber, int columnId,
                 int width, int height, bool rowIsSelected) override;

  virtual void cellDoubleClicked(int row, int col, const MouseEvent&) override;

  Connection *connection() { return _conn; }
  void set_connection(Connection *c) { _conn = c; }

  Controller *double_clicked_controller() { return _double_clicked_controller; }

private:
  Connection *_conn;
  Controller *_double_clicked_controller;
};


class CcMapsTableListBox : public KmTableListBox {
public:
  virtual void actionListenerCallback(const String &message) override;
};
