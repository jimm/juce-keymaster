#include <stdlib.h>
#include "patch.h"
#include "keymaster.h"

Patch::Patch(DBObjID id, const String &patch_name)
  : DBObj(id), Nameable(patch_name),
    _start_message(0), _stop_message(0),
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
}

bool Patch::is_running() {
  return _running;
}

void Patch::stop() {
  if (!_running)
    return;

  for (auto& conn : _connections)
    conn->stop();
  send_message_to_outputs(_stop_message);
  _running = false;
}

void Patch::add_connection(Connection *conn) {
  _connections.add(conn);
  if (is_running())
    conn->start();
}

void Patch::remove_connection(Connection *conn) {
  if (is_running())
    conn->stop();
  _connections.removeFirstMatchingValue(conn);
  delete conn;
}

void Patch::midi_in(MidiInput* source, const MidiMessage& message) {
  for (auto conn : _connections)
    conn->midi_in(source, message);
}

void Patch::send_message_to_outputs(MessageBlock *message) {
  if (message == nullptr)
    return;

  Array<String> output_ids;
  for (auto& conn : _connections)
    output_ids.addIfNotAlreadyThere(conn->output()->getIdentifier());

  for (auto output : KeyMaster_instance()->outputs())
    if (output_ids.contains(output->info.identifier))
      message->send_to(output);
}
