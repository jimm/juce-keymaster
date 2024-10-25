#include "keymaster.h"
#include "output.h"

Output::Output(MidiDeviceInfo device_info)
    : Instrument(device_info), device(MidiOutput::openDevice(info.identifier))
{
}

Output::~Output() {
  stop();
}

void Output::midi_out(MidiMessage &msg){
  if (device)
    device->sendMessageNow(msg);

  if (!_listeners.isEmpty())
    for (auto listener : _listeners)
      listener->midi_output(name(), msg);
}
