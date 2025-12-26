#pragma once

#include <JuceHeader.h>

#define CLOCK_LIGHT_SIZE 16
#define ON_COLOR Colours::lightgreen
#define OFF_COLOR Colours::darkgreen

class ClockLight : public Component, public ActionListener {
public:
  ClockLight() {
    _curr_color = OFF_COLOR;
  }

  virtual void paint(Graphics &g) override {
    g.setColour(_curr_color);
    g.fillEllipse(CLOCK_LIGHT_SIZE, CLOCK_LIGHT_SIZE / 4, CLOCK_LIGHT_SIZE, CLOCK_LIGHT_SIZE);
  }

  void actionListenerCallback(const String &message) override {
    if (message == "clock:beat") {
      _curr_color = ON_COLOR;
      repaint();
    }
    else if (message == "clock:unbeat") {
      _curr_color = OFF_COLOR;
      repaint();
    }
  }

private:
  Colour _curr_color;
};
