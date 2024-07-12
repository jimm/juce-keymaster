#pragma once

#include <JuceHeader.h>
#include "km_table_list_box.h"
#include "../km/keymaster.h"

class Patch;
class Trigger;
class TriggersTableListBox;


class TriggersTableListBoxModel : public KmTableListBoxModel {
public:
  void make_columns(TableHeaderComponent &header) override;

  int getNumRows() override { return KeyMaster_instance()->triggers().size(); }
  void paintCell(Graphics& g, int rowNumber, int columnId,
                 int width, int height, bool rowIsSelected) override;

private:
  String input_string(Trigger *t);
  String action_string(Trigger *t);
};


class TriggersTableListBox : public KmTableListBox {
public:
  void actionListenerCallback(const String &message) override {
    if (message == "moved") {
      updateContent();
      repaint();
    }
  }
};
