#pragma once

#include <JuceHeader.h>

class NotifyingComboBox : public ComboBox, public ActionBroadcaster {
public:
  virtual void valueChanged(Value &value) override { sendActionMessage("combo:changed"); }
};
