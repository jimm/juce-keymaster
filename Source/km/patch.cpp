#include <stdlib.h>
#include "patch.h"
#include "keymaster.h"

Patch::Patch(DBObjID id, const String &patch_name)
  : DBObj(id), Nameable(patch_name),
    _start_message(nullptr), _stop_message(nullptr),
    _running(false)
{
}

Patch::~Patch() {
  for (auto& conn : _connections)
    delete conn;
}

void Patch::start() {
  if (_running)
    return;

  send_message_to_outputs(_start_message);
  for (auto& conn : _connections)
    conn->start();
  _running = true;

  KeyMaster_instance()->cursor()->song()->start();
}

bool Patch::is_running() {
  return _running;
}

void Patch::stop() {
  if (!_running)
    return;

  KeyMaster_instance()->cursor()->song()->stop();

  for (auto& conn : _connections)
    conn->stop();
  send_message_to_outputs(_stop_message);
  _running = false;
}

void Patch::set_start_message(MessageBlock *msg) {
  if (_start_message != msg) {
    _start_message = msg;
    KeyMaster_instance()->changed();
  }
}

void Patch::set_stop_message(MessageBlock *msg) {
  if (_stop_message != msg) {
    _stop_message = msg;
    KeyMaster_instance()->changed();
  }
}

void Patch::add_connection(Connection *conn) {
  _connections.add(conn);
  if (is_running())
    conn->start();
  KeyMaster_instance()->changed();
}

void Patch::remove_connection(Connection *conn) {
  if (is_running())
    conn->stop();
  _connections.removeFirstMatchingValue(conn);
  delete conn;
  KeyMaster_instance()->changed();
}

void Patch::midi_in(Input::Ptr input, const MidiMessage& message) {
  for (auto conn : _connections)
    conn->midi_in(input, message);
}

void Patch::send_message_to_outputs(MessageBlock *message) {
  if (message == nullptr)
    return;

  Array<Output::Ptr> outputs;
  for (auto& conn : _connections)
    outputs.addIfNotAlreadyThere(conn->output());

  for (auto output : outputs)
    message->send_to(output);
}
