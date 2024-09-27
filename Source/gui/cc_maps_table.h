#pragma once

#include <JuceHeader.h>
#include "km_table_list_box.h"
#include "../km/keymaster.h"

class Connection;
class Controller;


class CcMapsTableListBoxModel : public KmTableListBoxModel {
public:
  void make_columns(TableHeaderComponent &header) override;

  int getNumRows() override { return _cc_maps.size(); }

  void paintCell(Graphics& g, int rowNumber, int columnId,
                 int width, int height, bool rowIsSelected) override;

  virtual void cellDoubleClicked(int row, int col, const MouseEvent&) override;

  void set_connection(Connection *c);
  Array<Controller *> &cc_maps() { return _cc_maps; }

private:
  Connection *_conn;
  Array<Controller *> _cc_maps;
};


class CcMapsTableListBox : public KmTableListBox {
public:
  void actionListenerCallback(const String &message) override {
    if (message == "moved") {
      updateContent();
      repaint();
    }
  }
};
