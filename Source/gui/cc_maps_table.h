#pragma once

#include <JuceHeader.h>
#include "km_table_list_box.h"
#include "../km/keymaster.h"

class Connection;
class Controller;


class CcMapsTableListBoxModel : public KmTableListBoxModel, public ActionListener {
public:
  void make_columns(TableHeaderComponent &header) override;

  Controller *nth_cc_map(int n);

  virtual int getNumRows() override;

  void paintCell(Graphics& g, int rowNumber, int columnId,
                 int width, int height, bool rowIsSelected) override;

  virtual void cellDoubleClicked(int row, int col, const MouseEvent&) override;

  void set_connection(Connection *c) { _conn = c; }

  virtual void actionListenerCallback(const String &message) override {
    if (message == CC_MAP_CHANGED_MESSAGE)
      sendActionMessage("update:cc-map");
  }

private:
  Connection *_conn;
};


class CcMapsTableListBox : public KmTableListBox {
public:
  void actionListenerCallback(const String &message) override {
    if (message == "moved" || message == "update:cc-map") {
      updateContent();
      repaint();
    }
  }
};
