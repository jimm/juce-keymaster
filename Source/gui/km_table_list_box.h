#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"


class KmTableListBoxModel : public TableListBoxModel {
public:
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
    Graphics &g, int _r, int _c, int _w, int _h, bool _selected) override
    {
      g.setColour(_lf.findColour(ListBox::textColourId));
      g.setFont(FontOptions(14.0f));
    }

  virtual int selected_row_num() { return -1; }

protected:
  LookAndFeel &_lf { LookAndFeel::getDefaultLookAndFeel() };

  Cursor *cursor() { return KeyMaster_instance()->cursor(); }
};


class KmTableListBox : public TableListBox, public ActionListener {
public:

  // void resized() override { setBoundsInset(BorderSize<int>(2)); }

  void actionListenerCallback(const String &message) override {
    if (message == "moved") {
      updateContent();
      int selected_row_num = ((KmTableListBoxModel *)getListBoxModel())->selected_row_num();
      if (selected_row_num >= 0 && selected_row_num <= getListBoxModel()->getNumRows())
        selectRow(selected_row_num);
      repaint();
    }
  }
};
