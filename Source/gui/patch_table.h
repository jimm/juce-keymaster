#pragma once

#include <JuceHeader.h>

class Patch;
class Connection;

class PatchTableListBoxModel : public Component, public TableListBoxModel {
public:
  PatchTableListBoxModel(Patch *p);

  int getNumRows() override { return _patch ? _patch->connections().size() : 0; }
  void paintRowBackground(Graphics& g, int rowNumber, int width, int height,
                          bool rowIsSelected) override;
  void paintCell(Graphics& g, int rowNumber, int columnId,
                 int width, int height, bool rowIsSelected) override;
  void resized() override;

private:
  Patch *_patch;
  TableListBox _table;

  String program_str(Connection *c);
  String controllers_str(Connection *c);
};
