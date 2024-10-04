#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"


class KmTableListBox;

class KmTableListBoxModel : public TableListBoxModel, public ActionBroadcaster {
public:
  virtual ~KmTableListBoxModel() {}

  virtual void make_columns(TableHeaderComponent &header) = 0;

  void paintRowBackground(
    Graphics& g, int rowNumber, int _width, int _height, bool rowIsSelected) override
    {
      if (rowIsSelected) {
        g.fillAll(Colours::lightblue);
        g.setColour(Colours::black);
      }
      else if (rowNumber % 2) {
        auto alternateColour = _lf.findColour(ListBox::backgroundColourId)
          .interpolatedWith(_lf.findColour(ListBox::textColourId), 0.1f);
        g.fillAll(alternateColour);
      }
    }

  void paintCell(
    Graphics &g, int _r, int _c, int _w, int _h, bool rowIsSelected) override
    {
      auto text_color = _lf.findColour(ListBox::textColourId);
      if (rowIsSelected)
        text_color = Colours::black;

      g.setColour(text_color);
      g.setFont(FontOptions(14.0f));
    }

  virtual int selected_row_num() { return -1; }

  virtual void cellClicked(int row, int col, const MouseEvent &event) override {
    if (event.mouseWasClicked() && event.mods.isRightButtonDown())
      sendActionMessage("popup");
    else
      TableListBoxModel::cellClicked(row, col, event);
  }

protected:
  LookAndFeel &_lf { LookAndFeel::getDefaultLookAndFeel() };

  Cursor *cursor() { return KeyMaster_instance()->cursor(); }
};


class KmTableListBox : public TableListBox, public ActionBroadcaster, public ActionListener {
public:
  virtual void actionListenerCallback(const String &message) override {
    if (message == "moved") {
      updateContent();
      auto model = static_cast<KmTableListBoxModel *>(getTableListBoxModel());
      int selected_row_num = model->selected_row_num();
      if (selected_row_num >= 0 && selected_row_num <= model->getNumRows())
        selectRow(selected_row_num);
      repaint();
    }
    else if (message == "update:table-list-box") {
      updateContent();
      repaint();
    }
    else if (message == "popup")
      popupMenu();
    else
      sendActionMessage(message);
  }

  virtual void backgroundClicked(const MouseEvent &event) override {
    if (event.mouseWasClicked() && event.mods.isRightButtonDown())
      popupMenu();
    else
      TableListBox::backgroundClicked(event);
  }

protected:
  virtual void popupMenu() {};
};
