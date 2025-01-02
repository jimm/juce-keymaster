#pragma once

#include <JuceHeader.h>
#include "km_list_box.h"
#include "../km/set_list.h"

class SetListsListBoxModel : public KmListBoxModel {
public:
  virtual ~SetListsListBoxModel() {}

  virtual int getNumRows() override {
    return set_lists().size();
  }

  virtual String &getText(int row) override {
    return set_lists()[row]->name();
  }

  virtual void selectedRowsChanged(int lastRowSelected) override {
    cursor()->jump_to_set_list_index(lastRowSelected);
  }

  virtual void listBoxItemDoubleClicked(int row, const MouseEvent&) override;

  int selected_row_num() override {
    return cursor()->set_list_index;
  }

private:
  Array<SetList *> &set_lists() { return KeyMaster_instance()->set_lists(); }
};

class SetListsListBox : public KmListBox {
public:
  virtual void actionListenerCallback(const String &message) override;

protected:
  virtual void popupMenu() override;
};
