#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"


class KmListBoxModel : public ListBoxModel {
public:
  virtual String &getText(int rowNumber) = 0;
  virtual int getNumRows() override { return 0; }

  void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override {
    if (rowIsSelected) {
      g.fillAll(Colours::lightblue);
      g.setColour(Colours::black);
    }
    else
      g.setColour(LookAndFeel::getDefaultLookAndFeel().findColour(ListBox::textColourId));

    g.setFont(14.0f);

    auto text = getText(rowNumber);
    g.drawText(text, 2, 0, width - 4, height, Justification::centredLeft, true);

    g.setColour(LookAndFeel::getDefaultLookAndFeel().findColour(ListBox::backgroundColourId));
    g.fillRect(width - 1, 0, 1, height);
  }

  virtual int selected_row_num() { return -1; }

protected:
  Cursor *cursor() { return KeyMaster_instance()->cursor(); }
};


class KmListBox : public ListBox, public ActionListener {
public:
  void actionListenerCallback(const String &message) override {
    if (message == "moved") {
      updateContent();
      int selected_row_num = ((KmListBoxModel *)getListBoxModel())->selected_row_num();
      if (selected_row_num >= 0 && selected_row_num <= getListBoxModel()->getNumRows())
        selectRow(selected_row_num);
      repaint();
    }
  }
};
