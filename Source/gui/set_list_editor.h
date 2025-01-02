#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"
#include "../km/set_list.h"
#include "km_editor.h"
#include "set_list_edit_box.h"
#include "set_list_model.h"

class AllSongsModel final : public SetListModel {
public:
  AllSongsModel() : SetListModel(KeyMaster_instance()->all_songs()) {}
};

class SetListEditor : public KmEditor, public DragAndDropContainer {
public:
  SetListEditor(SetList *sl, bool is_new);
  virtual ~SetListEditor() {}

  virtual int width() override;
  virtual int height() override;

private:
  SetList *_set_list;

  Label _name_label { {}, "Name" };
  TextEditor _name;

  Label _all_songs_label { {}, "All Songs" };
  ListBox _all_songs_box;
  AllSongsModel _all_songs_model;

  Label _set_list_label { {}, "Set List" };
  SetListEditBox _set_list_box;
  SetListEditModel _set_list_model { _set_list };

  virtual void layout(Rectangle<int> &area) override;
  void layout_name(Rectangle<int> &area);
  void layout_lists(Rectangle<int> &area);

  virtual void init() override;
  virtual void cancel_cleanup() override;

  virtual bool apply() override;
};

// If song is nullptr we create a new one.
SetListEditor * open_set_list_editor(SetList *sl);
