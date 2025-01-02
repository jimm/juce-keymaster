#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"
#include "../km/set_list.h"
#include "km_editor.h"

struct AllSongsModel final : public ListBoxModel {
  SetList * _all_songs = KeyMaster_instance()->all_songs();

  int getNumRows() override { return _all_songs->songs().size(); }

  void paintListBoxItem(int row, Graphics &g, int width, int height, bool rowIsSelected) override {
    if (rowIsSelected)
      g.fillAll(Colours::lightblue);

    g.setColour (LookAndFeel::getDefaultLookAndFeel().findColour(Label::textColourId));
    g.setFont((float)height * 0.7f);

    g.drawText(_all_songs->songs()[row]->name(),
               5, 0, width, height,
               Justification::centredLeft, true);
  }

  // TODO do I need this?
  var getDragSourceDescription (const SparseSet<int>& selected_rows) override {
    StringArray rows;
    for (int i = 0; i < selected_rows.size(); ++i)
      rows.add(_all_songs->songs()[selected_rows[i]]->name());
    return rows.joinIntoString (", ");
  }
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
  ListBox _all_songs;
  AllSongsModel _all_songs_model;

  virtual void layout(Rectangle<int> &area) override;
  void layout_name(Rectangle<int> &area);
  void layout_lists(Rectangle<int> &area);

  virtual void init() override;
  virtual void cancel_cleanup() override;

  virtual bool apply() override;
};

// If song is nullptr we create a new one.
SetListEditor * open_set_list_editor(SetList *sl);
