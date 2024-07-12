#pragma once

#include "instrument.h"

class Output : public Instrument {
public:

  using Ptr = ReferenceCountedObjectPtr<Output>;

  static Ptr find_by_id(String identifier);

  Output(MidiDeviceInfo info)
    : Instrument(info), device(MidiOutput::openDevice(info.identifier))
    {
    }

  void midi_out(MidiMessage &msg) { device->sendMessageNow(msg); }

private:
  std::unique_ptr<MidiOutput> device;
};
