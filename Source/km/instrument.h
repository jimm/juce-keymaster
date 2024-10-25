#pragma once

#include <JuceHeader.h>
#include "midi_listener.h"

class Instrument : public ReferenceCountedObject {
public:

  Instrument() {}               // for testing
  Instrument(MidiDeviceInfo mdi) : info(mdi) {}

  String name() { return info.name; }
  String identifier() { return info.identifier; }

  virtual void start() {}
  virtual void stop() {}
  virtual bool is_running() = 0;

  void add_listener(MidiListener *ml) { if (!_listeners.contains(ml)) _listeners.add(ml); }
  void remove_listener(MidiListener *ml) { _listeners.removeAllInstancesOf(ml); }

protected:
  Array<MidiListener *> _listeners;
  MidiDeviceInfo info;
};
