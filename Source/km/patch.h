#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "nameable.h"
#include "connection.h"
#include "message_block.h"

class Patch : public DBObj, public Nameable {
public:

  Patch(DBObjID id, const String &name);
  ~Patch();

  void start();
  bool is_running();
  void stop();

  inline Array<Connection *> &connections() { return _connections; }
  inline MessageBlock *start_message() { return _start_message; }
  inline MessageBlock *stop_message() { return _stop_message; }

  void set_start_message(MessageBlock *msg);
  void set_stop_message(MessageBlock *msg);

  void add_connection(Connection *conn);
  void remove_connection(Connection *conn);

  void midi_in(MidiInput* source, const MidiMessage& message);

private:
  Array<Connection *> _connections;
  MessageBlock *_start_message;
  MessageBlock *_stop_message;
  bool _running;

  void send_message_to_outputs(MessageBlock *message);
};
