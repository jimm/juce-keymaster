#include "keymaster.h"
#include "input.h"

Input::Ptr Input::find_by_id(String identifier) {
  for (auto &input : KeyMaster_instance()->inputs())
    if (input->identifier() == identifier)
      return input;
  return nullptr;
}
