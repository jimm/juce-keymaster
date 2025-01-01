#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"

class ClockButton : public TextButton, public ActionListener, public Button::Listener {
public:
  ClockButton() : TextButton("Start") {
    addListener(this);
  }

  virtual void buttonClicked(Button *) override {
    if (clock().is_running())
      clock().stop();
    else
      clock().start();
  }

  void actionListenerCallback(const String &message) override {
    // We don't have to start/stop the clock when we get "moved" because the
    // song takes care of that in its start/stop methods.
    if (message == "moved" || message == "clock:start" || message == "clock:stop")
      update();
  }

private:
  Clock &clock() { return KeyMaster_instance()->clock(); }

  void update() {
    bool running = clock().is_running();
    setToggleState(running, NotificationType::dontSendNotification);
    setButtonText(getToggleState() ? "Stop" : "Start");
  }
};
