#pragma once

#include <JuceHeader.h>

#define ON_COLOR Colours::lightgreen
#define OFF_COLOR Colours::darkgreen
#define X 12
#define Y 4
#define SIZE 16

class ClockLight : public Component, public ActionListener {
public:
  virtual void paint(Graphics &g) override {
    g.setColour(_curr_color);
    g.fillEllipse(X, Y, SIZE, SIZE);
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
  Colour _curr_color {OFF_COLOR};
};
