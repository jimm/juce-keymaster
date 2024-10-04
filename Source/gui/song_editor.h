#pragma once

#include <JuceHeader.h>
#include "../km/song.h"
#include "km_editor.h"

class Patch;

class SongEditor : public KmEditor {
public:
  SongEditor(Song *c, bool is_new);
  virtual ~SongEditor() {}

  virtual int width() override;
  virtual int height() override;

private:
  Song *_song;

  Label _name_label { {}, "Name" };
  TextEditor _name;

  Label _clock_label { {}, "Clock" };

  Label _bpm_label { {}, "BPM" };
  TextEditor _bpm;

  ToggleButton _clock_on { "Start clock when song starts" };

  Label _notes_label { {}, "Notes" };
  TextEditor _notes;

  virtual void layout(Rectangle<int> &area) override;
  void layout_name(Rectangle<int> &area);
  void layout_clock(Rectangle<int> &area);
  void layout_notes(Rectangle<int> &area);
  void layout_buttons(Rectangle<int> &area);

  virtual void init() override;
  virtual void cancel_cleanup() override;

  virtual bool apply() override;
};

// If song is nullptr we create a new one.
SongEditor * open_song_editor(Song *s);
