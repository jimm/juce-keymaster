#pragma once

#include "instrument.h"

class Input : public Instrument {
public:

  using Ptr = ReferenceCountedObjectPtr<Input>;

  static Ptr find_by_id(String identifier);

  Input(MidiDeviceInfo info, MidiInputCallback *listener)
    : Instrument(info), device(MidiInput::openDevice(info.identifier, listener))
    {
      device->start();
    }

private:
  std::unique_ptr<MidiInput> device;
};
