#include "keymaster.h"
#include "output.h"

Output::Output(MidiDeviceInfo info)
    : Instrument(info), device(MidiOutput::openDevice(info.identifier))
{
}

Output::~Output() {
  stop();
  device.reset();
}
