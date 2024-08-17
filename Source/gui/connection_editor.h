#pragma once

#include <JuceHeader.h>

class Connection;

class ConnectionEditor {
public:
  ConnectionEditor(Connection *conn, const char * const title, const char * const subtitle);

  void edit(ModalComponentManager::Callback *callback);

private:
  Connection *conn;

  AlertWindow *dialog;
};
