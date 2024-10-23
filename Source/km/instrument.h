#pragma once

class Instrument : public ReferenceCountedObject {
public:
  Instrument() {}               // for testing
  Instrument(MidiDeviceInfo mdi) : info(mdi) {}

  String name() { return info.name; }
  String identifier() { return info.identifier; }

  virtual void start() {}
  virtual void stop() {}
  virtual bool is_running() = 0;

protected:
  MidiDeviceInfo info;
};
