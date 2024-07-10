#pragma once

#include <JuceHeader.h>
#include "km_list_box.h"
#include "../km/trigger.h"

class TriggersListBox : public KmListBox {
};

class TriggersListBoxModel : public KmListBoxModel {
public:
  virtual int getNumRows() override {
    return triggers().size();
  }

  virtual String &getText(int row) override {
    return triggers()[row]->name();
  }

private:
  Array<Trigger *> &triggers() { return KeyMaster_instance()->triggers(); }
};
