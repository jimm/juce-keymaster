#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "input.h"
#include "nameable.h"
#include "connection.h"
#include "message_block.h"
#include "instrument_program_change.h"

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
  inline const Array<InstrumentProgramChange> &instrument_program_changes() const {
    return _instrument_program_changes;
  }

  void set_start_message(MessageBlock *msg);
  void set_stop_message(MessageBlock *msg);
  void add_instrument_program_change(InstrumentProgramChange ipc);
  void set_instrument_program_changes(Array<InstrumentProgramChange> ipcs);

  void add_connection(Connection *conn);
  void remove_connection(Connection *conn);

  void midi_in(Input::Ptr input, const MidiMessage& message);

private:
  Array<Connection *> _connections;
  Array<InstrumentProgramChange> _instrument_program_changes;
  MessageBlock *_start_message;
  MessageBlock *_stop_message;
  bool _running;

  void send_message_to_outputs(MessageBlock *message);
};
