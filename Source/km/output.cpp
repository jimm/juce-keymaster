#include "keymaster.h"
#include "output.h"

Output::Output(MidiDeviceInfo device_info)
    : Instrument(device_info), device(MidiOutput::openDevice(info.identifier))
{
}

Output::~Output() {
  stop();
}
