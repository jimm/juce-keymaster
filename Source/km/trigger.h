#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "message_block.h"
#include "nameable.h"

class KeyMaster;

typedef enum TriggerAction {
  TA_NEXT_SONG,
  TA_PREV_SONG,
  TA_NEXT_PATCH,
  TA_PREV_PATCH,
  TA_PANIC,
  TA_SUPER_PANIC,
  TA_TOGGLE_CLOCK,
  TA_MESSAGE
} TriggerAction;

class Trigger : public DBObj, public Nameable {
public:
  Trigger(DBObjID id, const String &name, TriggerAction action, MessageBlock *output);
  ~Trigger() {}

  inline int trigger_key_code() { return _trigger_key_code; }
  inline MidiMessage trigger_message() { return _trigger_message; }
  inline TriggerAction action() { return _action; }
  inline MessageBlock *output_message() { return _output_message; }

  inline void set_trigger_key_code(int key_code) { _trigger_key_code = key_code; }
         void set_trigger_message(MidiInput *input, MidiMessage message);
  inline void set_action(TriggerAction action) { _action = action; }
  inline void set_output_message(MessageBlock *msg) { _output_message = msg; }

  bool signal_message(MidiInput* source, const MidiMessage& message);
  bool signal_key(int key_code);

private:
  String _trigger_input_identifier;
  int _trigger_key_code;
  MidiMessage _trigger_message; // might be EMPTY_MESSAGE
  TriggerAction _action;
  MessageBlock *_output_message;
  int _trigger_message_num_bytes;

  void perform_action();
};
