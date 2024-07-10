#pragma once

#include <JuceHeader.h>
#include "clock.h"
#include "cursor.h"
#include "curve.h"
#include "message_block.h"
#include "set_list.h"
#include "trigger.h"
#include "midi_device.h"

class KeyMaster : private AudioDeviceManager, MidiInputCallback {
public:
  KeyMaster(bool testing=false);
  ~KeyMaster();

  // ================ accessors ================
  inline ReferenceCountedArray<MidiInputEntry> &inputs() { return _inputs; }
  inline ReferenceCountedArray<MidiOutputEntry> &outputs() { return _outputs; }
  inline SetList *all_songs() { return _set_lists[0]; }
  inline Array<SetList *> &set_lists() { return _set_lists; }
  inline Array<Trigger *> &triggers() { return _triggers; }
  inline Array<MessageBlock *> &messages() { return _messages; }
  inline Array<Curve *> &curves() { return _curves; }
  inline Cursor *cursor() { return _cursor; }
  inline Clock &clock() { return _clock; }
  inline bool is_testing() { return _testing; }

  void add_message(MessageBlock *message);
  void remove_message(MessageBlock *message);

  void add_curve(Curve *curve);
  void remove_curve(Curve *curve);
  Curve *curve_with_name(const String &name);
  Curve *curve_with_id(DBObjID id);

  void add_trigger(Trigger *trigger);
  void remove_trigger(Trigger *trigger);

  void add_set_list(SetList *set_list);
  void remove_set_list(SetList *set_list);

  void set_testing(bool val) { _testing = val; }

  // ================ running ================
  void start();
  void stop();
  void handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message) override;

  // ================ clock ================
  void start_clock() { _clock.start(); }
  void stop_clock() { _clock.stop(); }
  void toggle_clock() { if (is_clock_running()) _clock.stop(); else _clock.start(); }
  void set_clock_bpm(int bpm) { _clock.set_bpm(bpm); }
  bool is_clock_running() { return _clock.is_running(); }
  // Get BPM and start/stop from current song and update state of the clock
  void update_clock();

  // ================ initialization ================
  void initialize();
  void load_instruments();

  // ================ movement ================
  void next_patch();
  void prev_patch();
  void next_song();
  void prev_song();

  // ================ going places ================
  void goto_song(Song *song);
  void goto_patch(Patch *patch);

  void goto_song(const String &name_regex);
  void goto_set_list(const String &name_regex);

  void jump_to_set_list_index(int i);
  void jump_to_song_index(int i);
  void jump_to_patch_index(int i);

  // ================ doing things ================
  void panic(bool send_notes_off);

  // ================ helpers ================
  void sort_all_songs();

private:
  ReferenceCountedArray<MidiInputEntry> _inputs;
  ReferenceCountedArray<MidiOutputEntry> _outputs;
  Array<Trigger *> _triggers;
  Array<SetList *> _set_lists; // all set lists, including all_songs
  Cursor *_cursor;
  Clock _clock;
  bool _running;
  bool _testing;
  Array<MessageBlock *> _messages;
  Array<Curve *> _curves;

  // ================ initialization ================
  void create_songs();
};



KeyMaster *KeyMaster_instance();
