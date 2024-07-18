#pragma once

class Instrument : public ReferenceCountedObject {
public:
  MidiDeviceInfo info;

  Instrument(MidiDeviceInfo mdi) : info(mdi) {}

  virtual void start() {}
  virtual void stop() {}
  virtual bool is_running() = 0;
};
