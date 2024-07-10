#pragma once

#include <JuceHeader.h>
#include "km_list_box.h"
#include "../km/message_block.h"

class MessagesListBox : public KmListBox {
};

class MessagesListBoxModel : public KmListBoxModel {
public:
  virtual int getNumRows() override {
    return messages().size();
  }

  virtual String &getText(int row) override {
    return messages()[row]->name();
  }

private:
  Array<MessageBlock *> &messages() { return KeyMaster_instance()->messages(); }
};
