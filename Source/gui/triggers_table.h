#pragma once

#include <JuceHeader.h>
#include "km_table_list_box.h"
#include "../km/keymaster.h"

class TriggersTableListBoxModel : public KmTableListBoxModel {
public:
  virtual ~TriggersTableListBoxModel() {}

  void make_columns(TableHeaderComponent &header) override;

  int getNumRows() override { return KeyMaster_instance()->triggers().size(); }
  void paintCell(Graphics& g, int rowNumber, int columnId,
                 int width, int height, bool rowIsSelected) override;

  virtual void selectedRowsChanged(int lastRowSelected) override {
    cursor()->jump_to_trigger_index(lastRowSelected);
  }

  int selected_row_num() override { return cursor()->trigger_index; }

  virtual void cellDoubleClicked(int row, int col, const MouseEvent&) override;

private:
  String action_string(Trigger *t);
};


class TriggersTableListBox : public KmTableListBox {
public:
  virtual void actionListenerCallback(const String &message) override;

protected:
  virtual void popupMenu() override;
};
