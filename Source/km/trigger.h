#pragma once

#include <JuceHeader.h>
#include "db_obj.h"
#include "input.h"
#include "message_block.h"
#include "nameable.h"
#include "utils.h"

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

class Trigger : public DBObj, public Nameable, public KeyListener {
public:
  Trigger(DBObjID id, const String &name, TriggerAction action, MessageBlock *output);
  virtual ~Trigger() override {}

  inline bool has_trigger_key_press() { return _trigger_key_press.isValid(); }
  inline KeyPress & trigger_key_press() { return _trigger_key_press; }
  inline bool has_trigger_message() { return !mm_equal(_trigger_message, EMPTY_MESSAGE); }
  inline MidiMessage trigger_message() { return _trigger_message; }
  inline TriggerAction action() { return _action; }
  inline MessageBlock *output_message() { return _output_message; }

  // To erase trigger key press, pass in KeyPress() with no key code
  void set_trigger_key_press(const KeyPress key_press);
  // To erase trigger message, pass in EMPTY_MESSAGE
  void set_trigger_message(MidiMessage message);
  void set_action(TriggerAction action);
  void set_output_message(MessageBlock *msg);

  bool signal_message(const MidiMessage& message);

  virtual bool keyPressed(const KeyPress &key, Component *) override;

private:
  KeyPress _trigger_key_press;
  MidiMessage _trigger_message; // might be EMPTY_MESSAGE
  TriggerAction _action;
  MessageBlock *_output_message;

  void perform_action();
};
