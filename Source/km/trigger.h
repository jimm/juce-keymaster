#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "message_block.h"
#include "nameable.h"

class KeyMaster;

typedef enum TriggerAction {
  NEXT_SONG,
  PREV_SONG,
  NEXT_PATCH,
  PREV_PATCH,
  PANIC,
  SUPER_PANIC,
  TOGGLE_CLOCK,
  MESSAGE
} TriggerAction;

class Trigger : public DBObj, public Nameable {
public:
  Trigger(DBObjID id, const String &name, TriggerAction action, MessageBlock *output);
  ~Trigger() {}

  inline String trigger_input_identifier() { return _trigger_input_identifier; }
  inline int trigger_key_code() { return _trigger_key_code; }
  inline MidiMessage trigger_message() { return _trigger_message; }
  inline TriggerAction action() { return _action; }
  inline MessageBlock *output_message() { return _output_message; }

  inline void set_trigger_key_code(int key_code) { _trigger_key_code = key_code; }
  // To erase trigger message, make input == nullptr
         void set_trigger_message(String input_identifier, MidiMessage message);
  inline void set_action(TriggerAction action) { _action = action; }
  inline void set_output_message(MessageBlock *msg) { _output_message = msg; }

  bool signal_message(MidiInput* source, const MidiMessage& message);
  bool signal_key(int key_code);

private:
  String _trigger_input_identifier;
  int _trigger_key_code;        // might be UNDEFINED
  MidiMessage _trigger_message; // might be EMPTY_MESSAGE
  TriggerAction _action;
  MessageBlock *_output_message;
  int _trigger_message_num_bytes;

  void perform_action();
};
