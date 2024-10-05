#include <JuceHeader.h>
#include "utils.h"

bool mm_equal(const MidiMessage &a, const MidiMessage &b) {
  int a_size = a.getRawDataSize();
  if (a_size != b.getRawDataSize())
    return false;
  return (std::memcmp(a.getRawData(), b.getRawData(), (size_t)a_size) == 0);
}
