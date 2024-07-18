#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "input.h"
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

  inline Input::Ptr trigger_input() { return _trigger_input; }
  inline int trigger_key_code() { return _trigger_key_code; }
  inline MidiMessage trigger_message() { return _trigger_message; }
  inline TriggerAction action() { return _action; }
  inline MessageBlock *output_message() { return _output_message; }

  void set_trigger_key_code(int key_code);
  // To erase trigger message, make input == nullptr
  void set_trigger_message(Input::Ptr input, MidiMessage message);
  void set_action(TriggerAction action);
  void set_output_message(MessageBlock *msg);

  bool signal_message(Input::Ptr input, const MidiMessage& message);
  bool signal_key(int key_code);

private:
  Input::Ptr _trigger_input;    // might be nullptr
  int _trigger_key_code;        // might be UNDEFINED
  MidiMessage _trigger_message; // might be EMPTY_MESSAGE
  TriggerAction _action;
  MessageBlock *_output_message;
  int _trigger_message_num_bytes;

  void perform_action();
};
