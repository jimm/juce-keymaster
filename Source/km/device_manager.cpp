#include "device_manager.h"
#include "keymaster.h"

DeviceManager::~DeviceManager() {
  _inputs.clear();
  _outputs.clear();
}

void DeviceManager::handleIncomingMidiMessage(MidiInput *source, const MidiMessage &message) {
  Input::Ptr input = find_input(source->getIdentifier());
  input->midi_in(message);
}

void DeviceManager::clear() {
  _inputs.clear();
  _identifier_to_input.clear();
  _outputs.clear();
  _identifier_to_output.clear();
}

void DeviceManager::update_devices() {
  clear();
  update_inputs();
  update_outputs();
}

void DeviceManager::update_inputs() {
// Don't load real MIDI devices when running tests
#ifndef JUCE_UNIT_TESTS
  for (auto info : MidiInput::getAvailableDevices())
    find_or_create_input(info)->start();
#endif
}

void DeviceManager::update_outputs() {
// Don't load real MIDI devices when running tests
#ifndef JUCE_UNIT_TESTS
  for (auto info : MidiOutput::getAvailableDevices())
    find_or_create_output(info);
#endif
}

Input::Ptr DeviceManager::find_input(const String &identifier) {
  if (_identifier_to_input.contains(identifier))
    return _identifier_to_input[identifier];
  return nullptr;
}

Input::Ptr DeviceManager::find_or_create_input(const String &identifier, const String &name) {
  return find_or_create_input(MidiDeviceInfo(name, identifier));
}

Input::Ptr DeviceManager::find_or_create_input(const MidiDeviceInfo info) {
  Input::Ptr input = find_input(info.identifier);
  if (input == nullptr) {
    // Look for the name, keeping the identifier we have because that's the
    // current system's identifier for that name.
    for (auto [id, inp] : _identifier_to_input) {
      if (inp->name() == info.name) {
        input = inp;
        break;
      }
    }
  }
  if (input == nullptr) {
    input = new Input(info, this);
    _inputs.add(input);
    _identifier_to_input[info.identifier] = input;
  }
  return input;
}

Output::Ptr DeviceManager::find_output(const String &identifier) {
  if (_identifier_to_output.contains(identifier))
    return _identifier_to_output[identifier];
  return nullptr;
}

Output::Ptr DeviceManager::find_or_create_output(const String &identifier, const String &name) {
  return find_or_create_output(MidiDeviceInfo(name, identifier));
}

Output::Ptr DeviceManager::find_or_create_output(const MidiDeviceInfo info) {
  Output::Ptr output = find_output(info.identifier);
  // Look for the name, keeping the identifier we have because that's the
  // current system's identifier for that name.
  for (auto [id, outp] : _identifier_to_output) {
    if (outp->name() == info.name) {
      output = outp;
      break;
    }
  }
  if (output == nullptr) {
    output = new Output(info);
    _outputs.add(output);
    _identifier_to_output[info.identifier] = output;
  }
  return output;
}
