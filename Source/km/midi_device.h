#pragma once

struct MidiInputEntry final : ReferenceCountedObject {
  explicit MidiInputEntry(MidiDeviceInfo info, MidiInputCallback *listener)
    : info(info), device(MidiInput::openDevice(info.identifier, listener))
    {
      device->start();
    }

  MidiDeviceInfo info;
  std::unique_ptr<MidiInput> device;

  using Ptr = ReferenceCountedObjectPtr<MidiInputEntry>;
};

struct MidiOutputEntry final : ReferenceCountedObject {
  explicit MidiOutputEntry(MidiDeviceInfo info)
    : info(info), device(MidiOutput::openDevice(info.identifier))
    {
    }

  MidiDeviceInfo info;
  std::unique_ptr<MidiOutput> device;

  using Ptr = ReferenceCountedObjectPtr<MidiOutputEntry>;
};
