#pragma once

#include <JuceHeader.h>
#include "../km/song.h"
#include "km_table_list_box.h"

class Patch;

class SongDialogComponent : public Component, public ActionBroadcaster {
public:
  SongDialogComponent(Song *c, bool is_new);

  void resized() override;

  int width();
  int height();

private:
  Song *_song;
  bool _is_new;

  Label _name_label { {}, "Name" };
  TextEditor _name;

  Label _clock_label { {}, "Clock" };

  Label _bpm_label { {}, "BPM" };
  TextEditor _bpm;

  ToggleButton _clock_on { "Start clock when song starts" };

  Label _notes_label { {}, "Notes" };
  TextEditor _notes;

  TextButton _ok { "Ok" };
  TextButton _cancel { "Cancel" };
  TextButton _apply { "Apply" };

  void layout_name(Rectangle<int> &area);
  void layout_clock(Rectangle<int> &area);
  void layout_notes(Rectangle<int> &area);
  void layout_buttons(Rectangle<int> &area);

  void init_song();

  void ok();
  void cancel();
  bool apply();
};

// If song is nullptr we create a new one.
SongDialogComponent * open_song_editor(Song *s);
