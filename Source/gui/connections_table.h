#pragma once

#include <JuceHeader.h>
#include "km_table_list_box.h"
#include "../km/keymaster.h"

class Patch;
class Connection;
class ConnectionsTableListBox;


class ConnectionsTableListBoxModel : public KmTableListBoxModel {
public:
  void make_columns(TableHeaderComponent &header) override;

  int getNumRows() override {
    Patch *p = patch();
    return p ? p->connections().size() : 0;
  }
  void paintCell(Graphics& g, int rowNumber, int columnId,
                 int width, int height, bool rowIsSelected) override;

private:
  Patch *patch() { return cursor()->patch(); }
  String program_str(Connection *c);
  String controllers_str(Connection *c);
};


class ConnectionsTableListBox : public KmTableListBox {
public:
  void actionListenerCallback(const String &_message) override {
    updateContent();
    repaint();
  }
};
