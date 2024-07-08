#pragma once

struct MidiInputEntry final : ReferenceCountedObject {
  explicit MidiInputEntry(MidiDeviceInfo info, MidiInputCallback *listener)
    : info(info), device(MidiInput::openDevice(info.identifier, listener).get())
    {
      device->start();
    }

  MidiDeviceInfo info;
  MidiInput *device;

  using Ptr = ReferenceCountedObjectPtr<MidiInputEntry>;
};

struct MidiOutputEntry final : ReferenceCountedObject {
  explicit MidiOutputEntry(MidiDeviceInfo info)
    : info(info), device(MidiOutput::openDevice(info.identifier).get())
    {
    }

  MidiDeviceInfo info;
  MidiOutput *device;

  using Ptr = ReferenceCountedObjectPtr<MidiOutputEntry>;
};
