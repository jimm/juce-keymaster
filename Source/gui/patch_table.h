#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"

class Patch;
class Connection;

class PatchTableListBoxModel : public Component, public TableListBoxModel {
public:
  PatchTableListBoxModel();

  int getNumRows() override {
    Patch *p = patch();
    return p ? p->connections().size() : 0;
  }
  void paintRowBackground(Graphics& g, int rowNumber, int width, int height,
                          bool rowIsSelected) override;
  void paintCell(Graphics& g, int rowNumber, int columnId,
                 int width, int height, bool rowIsSelected) override;
  void resized() override;

private:
  TableListBox _table;

  Patch *patch() { return KeyMaster_instance()->cursor()->patch(); }
  String program_str(Connection *c);
  String controllers_str(Connection *c);
};
