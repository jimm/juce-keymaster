#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"

class ClockBpm : public TextEditor, public ActionListener, public TextEditor::Listener {
public:
  virtual void textEditorTextChanged(TextEditor &) override {
    clock().set_bpm(getText().getFloatValue());
  }

  void update_contents() {
    setText(String(clock().bpm()), NotificationType::dontSendNotification);
    repaint();
  }

  void actionListenerCallback(const String &message) override {
    if (message == "clock:bpm")
      update_contents();
  }

private:
  Clock &clock() { return KeyMaster_instance()->clock(); }
};
