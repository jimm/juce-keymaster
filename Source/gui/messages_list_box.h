#pragma once

#include <JuceHeader.h>
#include "km_list_box.h"
#include "../km/message_block.h"

class MessagesListBox : public KmListBox {
};

class MessagesListBoxModel : public KmListBoxModel {
public:
  virtual ~MessagesListBoxModel() {}

  virtual int getNumRows() override {
    return messages().size();
  }

  virtual String &getText(int row) override {
    return messages()[row]->name();
  }

  virtual void selectedRowsChanged(int lastRowSelected) override {
    cursor()->jump_to_message_index(lastRowSelected);
  }

  int selected_row_num() override { return cursor()->message_index; }

private:
  Array<MessageBlock *> &messages() { return KeyMaster_instance()->messages(); }
};
