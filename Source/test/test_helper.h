#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"
#include "../km/device_manager.h"

// JUCE MidiMessage channels start at 1
#define JCH(n) ((n)+1)

extern KeyMaster *load_test_data(DeviceManager &dev_mgr, String data_file_path = "");

extern bool mm_eq(MidiMessage a, MidiMessage b);
