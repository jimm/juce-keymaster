#pragma once

#include <JuceHeader.h>

// ================

class NameableListBoxModel : public ListBoxModel {
public:
  virtual String &getText(int rowNumber) = 0;
  virtual int getNumRows() override { return 0; }

  void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override {
    if (rowIsSelected) {
      g.fillAll(Colours::lightblue);
      g.setColour(Colours::black);
    }
    else
      g.setColour(LookAndFeel::getDefaultLookAndFeel().findColour(ListBox::textColourId));

    g.setFont(14.0f);

    auto text = getText(rowNumber);
    g.drawText(text, 2, 0, width - 4, height, Justification::centredLeft, true);

    g.setColour(LookAndFeel::getDefaultLookAndFeel().findColour(ListBox::backgroundColourId));
    g.fillRect(width - 1, 0, 1, height);
  }
};

// ================

#include "../km/set_list.h"

class SetListSongsListBoxModel : public NameableListBoxModel {
public:
  SetListSongsListBoxModel(SetList *list) : _set_list(list) {}

  virtual int getNumRows() override { return _set_list ? _set_list->songs().size() : 0; }
  virtual String &getText(int row) override { return _set_list->songs()[row]->name(); }

private:
  SetList *_set_list;
};

// ================

#include "../km/song.h"

class SongPatchesListBoxModel : public NameableListBoxModel {
public:
  SongPatchesListBoxModel(Song *song) : _song(song) {}

  virtual int getNumRows() override { return _song ? _song->patches().size() : 0; }
  virtual String &getText(int row) override { return _song->patches()[row]->name(); }

private:
  Song *_song;
};

// ================

class SetListsListBoxModel : public NameableListBoxModel {
public:
  SetListsListBoxModel(Array<SetList *> &list) : _set_lists(list) {}

  virtual int getNumRows() override { return _set_lists.size(); }
  virtual String &getText(int row) override { return _set_lists[row]->name(); }

private:
  Array<SetList *> &_set_lists;
};

// ================

#include "../km/message_block.h"

class MessagesListBoxModel : public NameableListBoxModel {
public:
  MessagesListBoxModel(Array<MessageBlock *> &list) : _messages(list) {}

  virtual int getNumRows() override { return _messages.size(); }
  virtual String &getText(int row) override { return _messages[row]->name(); }

private:
  Array<MessageBlock *> &_messages;
};

// ================

#include "../km/trigger.h"

class TriggersListBoxModel : public NameableListBoxModel {
public:
  TriggersListBoxModel(Array<Trigger *> &list) : _triggers(list) {}

  virtual int getNumRows() override { return _triggers.size(); }
  virtual String &getText(int row) override { return _triggers[row]->name(); }

private:
  Array<Trigger *> &_triggers;
};
