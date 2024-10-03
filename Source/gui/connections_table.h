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

  virtual void selectedRowsChanged(int lastRowSelected) override {
    cursor()->jump_to_connection_index(lastRowSelected);
  }

  int selected_row_num() override { return cursor()->connection_index; }

  virtual void cellDoubleClicked(int row, int col, const MouseEvent&) override;

  virtual void actionListenerCallback(const String &message) override {
    if (message == CONNECTION_CHANGED_MESSAGE)
      sendActionMessage("update:table-list-box");
    else
      KmTableListBoxModel::actionListenerCallback(message);
  }

private:
  Patch *patch() { return cursor()->patch(); }
  String program_str(Connection *c);
  String controllers_str(Connection *c);
};


class ConnectionsTableListBox : public KmTableListBox {
protected:
  virtual void popupMenu() override;
};
