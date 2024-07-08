#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "nameable.h"

class Curve : public DBObj, public Nameable {
public:
  String _short_name;
  unsigned char curve[128];

  Curve(DBObjID id, const String &name, const String &short_name);

  String short_name() { return _short_name; }

protected:
  virtual void generate() { }
};

extern void generate_default_curves(Array<Curve *> &vec);
