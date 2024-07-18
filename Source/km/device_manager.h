#pragma once

#include <JuceHeader.h>
#include "input.h"
#include "output.h"

class DeviceManager : MidiInputCallback {
public:

  ~DeviceManager();

  void update_devices();

  ReferenceCountedArray<Input> &inputs() { return _inputs; }
  ReferenceCountedArray<Output> &outputs() { return _outputs; }

  Input::Ptr find_input(const String &identifier);
  Input::Ptr find_or_create_input(const String &identifier, const String &name);
  Input::Ptr find_or_create_input(const MidiDeviceInfo &info);

  Output::Ptr find_output(const String &identifier);
  Output::Ptr find_or_create_output(const String &identifier, const String &name);
  Output::Ptr find_or_create_output(const MidiDeviceInfo &info);

private:
  ReferenceCountedArray<Input> _inputs;
  std::map<String, Input::Ptr> _identifier_to_input;
  ReferenceCountedArray<Output> _outputs;

  MidiDeviceListConnection connection = MidiDeviceListConnection::make(
    [this] { update_devices(); });

  void handleIncomingMidiMessage (MidiInput *source, const MidiMessage &message) override;

  void update_inputs();
  void update_outputs();
};
