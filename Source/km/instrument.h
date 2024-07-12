#pragma once

class Instrument : public ReferenceCountedObject {
public:
  Instrument(MidiDeviceInfo info) : _info(info) {}

  String identifier() { return _info.identifier; }
  String name() { return _info.name; }

protected:
  MidiDeviceInfo _info;
};
