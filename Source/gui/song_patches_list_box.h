#pragma once

#include <JuceHeader.h>
#include "km_list_box.h"
#include "../km/song.h"

class SongPatchesListBoxModel : public KmListBoxModel {
public:
  virtual ~SongPatchesListBoxModel() {}

  virtual int getNumRows() override {
    Song *s = song();
    return s ? s->patches().size() : 0;
  }

  virtual String &getText(int row) override {
    return song()->patches()[row]->name();
  }

  virtual void selectedRowsChanged(int lastRowSelected) override {
    cursor()->jump_to_patch_index(lastRowSelected);
  }

  virtual void listBoxItemDoubleClicked(int row, const MouseEvent&) override;

  int selected_row_num() override { return cursor()->patch_index; }

private:
  Song *song() { return cursor()->song(); }
};

class SongPatchesListBox : public KmListBox {
public:
  virtual void actionListenerCallback(const String &message) override;

protected:
  virtual void popupMenu() override;
};
