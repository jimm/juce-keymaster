#pragma once

#include <JuceHeader.h>

class ClockLight : public ImageButton, public ActionListener {
public:
  ClockLight() {
    Graphics g(_image);
    g.setColour(Colours::lightgreen);
    g.fillEllipse(0, 0, 16, 16);

    setImages(false, false, true,
              _image, 0.4f, Colours::transparentBlack,
              _image, 1.0f, Colours::transparentBlack,
              _image, 1.0f, Colours::transparentBlack,
              0.5f);
  }

  void actionListenerCallback(const String &message) override {
    if (message == "clock:beat")
      setState(ButtonState::buttonDown);
    else if (message == "clock:unbeat")
      setState(ButtonState::buttonNormal);
  }

private:
  Image _image { Image::RGB, 16, 16, false };
};
