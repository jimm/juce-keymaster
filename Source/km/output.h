#pragma once

#include "instrument.h"

class Output : public Instrument {
public:

  using Ptr = ReferenceCountedObjectPtr<Output>;

  Output() {}                   // for testing
  Output(MidiDeviceInfo device_info);
  virtual ~Output() override;

  bool is_running() override { return (bool)device; }

  void midi_out(MidiMessage &msg);

private:
  std::unique_ptr<MidiOutput> device;
};
