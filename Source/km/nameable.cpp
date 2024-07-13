#include "keymaster.h"
#include "nameable.h"

void Nameable::set_name(const String &str) {
  if (_name != str) {
    _name = str;
    KeyMaster_instance()->changed();
  }
}
