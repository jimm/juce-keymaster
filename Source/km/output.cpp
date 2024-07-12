#include "keymaster.h"
#include "output.h"

Output::Ptr Output::find_by_id(String identifier) {
  for (auto &output : KeyMaster_instance()->outputs())
    if (output->identifier() == identifier)
      return output;
  return nullptr;
}
