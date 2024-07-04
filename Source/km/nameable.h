#pragma once

#include <JuceHeader.h>

class Nameable {
public:
  Nameable(const String &str) : _name(str) {}
  virtual ~Nameable() {}

  inline String &name() { return _name; }
  void set_name(const String &str) { _name = str; }

private:
  String _name;
};

class NameableSorter {
public:
  int compareElements(Nameable *s1, Nameable *s2) {
    return s1->name().compareNatural(s2->name());
  }
};
