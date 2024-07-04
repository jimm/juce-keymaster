#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "patch.h"

class Song : public DBObj, public Nameable {
public:
  Song(DBObjID id, const String &name);
  ~Song();

  inline Array<Patch *> &patches() { return _patches; }
  inline String &notes() { return _notes; }
  inline float bpm() { return _bpm; }
  inline bool clock_on_at_start() { return _clock_on_at_start; }

  inline void set_notes(const String &notes) { _notes = notes; }
  inline void set_bpm(float bpm) { _bpm = bpm; }
  inline void set_clock_on_at_start(bool val) { _clock_on_at_start = val; }

  void add_patch(Patch *patch);
  void remove_patch(Patch *patch);

private:
  Array<Patch *> _patches;
  String _notes;
  float _bpm;
  bool _clock_on_at_start;
};
