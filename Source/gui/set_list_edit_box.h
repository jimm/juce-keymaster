#pragma once

#include <JuceHeader.h>
#include "set_list_model.h"

class EditSetList : public SetList {
public:
  EditSetList(SetList *other) : SetList(UNDEFINED_ID, "editing") {
    _songs = other->songs();
  }

  void move_song(int new_index, int old_index) {
    _songs.move(old_index, new_index);
  }

  void insert_songs(int index, Song **songs, int num_songs) {
    _songs.insertArray(index, songs, num_songs);
  }

  void remove_song_at(int index) {
    _songs.remove(index);
  }
};

class SetListEditModel final : public SetListModel {
public:
  SetListEditModel(SetList *orig_set_list) : SetListModel(new EditSetList(orig_set_list)) {}

  ~SetListEditModel() {
    delete _set_list;
  }
};

class SetListEditBox final : public ListBox, public DragAndDropTarget {
public:

  void paintOverChildren(Graphics &g) override {
    ListBox::paintOverChildren(g);
    if (!_dragging)
      return;

    g.setColour(Colours::green);
    g.drawRect(getLocalBounds(), 3);

    int row = getInsertionIndexForPosition(_drag_pos.x, _drag_pos.y);
    auto rect = getRowPosition(row, true);
    g.drawRect(rect.removeFromTop(3), 3);
  }

  bool isInterestedInDragSource(const SourceDetails &details) override {
    return true;
  }

  void itemDragEnter(const SourceDetails &details) override {
    _dragging = true;
    _drag_pos = details.localPosition;
    repaint();
  }

  void itemDragMove(const SourceDetails &details) override {
    if (_dragging) {
      _drag_pos = details.localPosition;
      repaint();
    }
  }

  void itemDragExit(const SourceDetails &details) override {
    _dragging = false;
    repaint();
  }

  void itemDropped(const SourceDetails &details) override {
    auto model = static_cast<SetListEditModel *>(getListBoxModel());
    auto edit_set_list = static_cast<EditSetList *>(model->set_list());
    bool from_this_box = details.sourceComponent == this;
    int row = getInsertionIndexForPosition(_drag_pos.x, _drag_pos.y);

    if (from_this_box) {
      // We'll only ever have one song to move because the editor sets this
      // list box to disallow multiple selection.
      Array<var> *arr = details.description.getArray();
      int dropped_song_index = int((*arr)[0]);
      edit_set_list->move_song(row, dropped_song_index);
    }
    else {
      Array<Song *> &source_songs = KeyMaster_instance()->all_songs()->songs();
      Array<Song *> dropped_songs;
      for (auto v : *details.description.getArray())
        dropped_songs.add(source_songs[int(v)]);
      edit_set_list->insert_songs(row, dropped_songs.getRawDataPointer(), dropped_songs.size());
    }

    _dragging = false;
    repaint();
  }

  bool shouldDrawDragImageWhenOver() override {
    return true;
  }

private:
  bool _dragging;
  Point<int> _drag_pos;
};
