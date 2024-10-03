#pragma once

#include <JuceHeader.h>
#include "km_list_box.h"
#include "../km/set_list.h"

class SetListSongsListBox : public KmListBox {
protected:
  virtual void popupMenu() override;

  void popup_all_songs_menu();
  void popup_set_list_menu();
};

class SetListSongsListBoxModel : public KmListBoxModel {
public:
  virtual ~SetListSongsListBoxModel() {}

  virtual int getNumRows() override {
    return set_list()->songs().size();
  }

  virtual String &getText(int row) override {
    return set_list()->songs()[row]->name();
  }

  virtual void selectedRowsChanged(int lastRowSelected) override {
    cursor()->jump_to_song_index(lastRowSelected);
  }

  void listBoxItemDoubleClicked(int row, const MouseEvent&) override;

  int selected_row_num() override { return cursor()->song_index; }

  SetList *set_list() { return cursor()->set_list(); }
};
