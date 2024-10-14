#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"


class KmListBox;

class KmListBoxModel : public ListBoxModel, public ActionBroadcaster {
public:
  virtual ~KmListBoxModel() {}

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

  virtual void listBoxItemClicked(int row, const MouseEvent &event) override {
    if (event.mouseWasClicked() && event.mods.isRightButtonDown())
      sendActionMessage("popup");
    else
      ListBoxModel::listBoxItemClicked(row, event);
  }

protected:
  Cursor *cursor() { return KeyMaster_instance()->cursor(); }
};


class KmListBox : public ListBox, public ActionBroadcaster, public ActionListener {
public:
  virtual void actionListenerCallback(const String &message) override {
    if (message == "moved") {
      updateContent();
      int selected_row_num = ((KmListBoxModel *)getListBoxModel())->selected_row_num();
      if (selected_row_num >= 0 && selected_row_num <= getListBoxModel()->getNumRows())
        selectRow(selected_row_num);
      repaint();
    }
    else if (message == "update:list-box") {
      updateContent();
      repaint();
    }
    else if (message == "popup")
      popupMenu();
    else
      sendActionMessage(message);
  }

  virtual void mouseUp(const MouseEvent &event) override {
    if (event.mouseWasClicked() && event.mods.isRightButtonDown())
      popupMenu();
    else
      ListBox::mouseUp(event);
  }

protected:
  virtual void popupMenu() {};
};
