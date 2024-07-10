#pragma once

#include <JuceHeader.h>
#include "km_list_box.h"
#include "../km/set_list.h"

class SetListSongsListBox : public KmListBox {
protected:
};

class SetListSongsListBoxModel : public KmListBoxModel {
public:
  virtual int getNumRows() override {
    return set_list()->songs().size();
  }

  virtual String &getText(int row) override {
    return set_list()->songs()[row]->name();
  }

  virtual void listBoxItemClicked(int row, const MouseEvent &_) override {
    cursor()->jump_to_song_index(row);
  }

  int selected_row_num() override { return cursor()->song_index; }

private:
  SetList *set_list() { return cursor()->set_list(); }
};
