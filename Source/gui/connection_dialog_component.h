#pragma once

#include <JuceHeader.h>
#include "../km/connection.h"

class Patch;

class ConnectionDialogComponent : public Component {
public:
  ConnectionDialogComponent(Patch *p, Connection *c);

  void resized() override;

private:
  Patch *_patch;
  Connection *_conn;
  Connection _e_conn;
  TextButton _ok { "Ok" };
  TextButton _cancel { "Cancel" };
  TextButton _apply { "Apply" };

  void ok();
  void cancel();
  void apply();
};
