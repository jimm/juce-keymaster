#pragma once

#include <JuceHeader.h>
#include "../km/trigger.h"
#include "../km/midi_message_learner.h"
#include "km_editor.h"

class KeyLearner : public Label, public ActionBroadcaster {
public:
  KeyLearner();

  void start_learning();
  KeyPress & key() { return _key; }
  void set_key(const KeyPress &key);

  virtual bool keyPressed(const KeyPress &key) override;

private:
  bool _learning;
  KeyPress _key;
};

class TriggerEditor : public KmEditor, public MidiMessageLearner, public ActionListener {
public:
  TriggerEditor(Trigger *t, bool is_new);
  virtual ~TriggerEditor();

  virtual int width() override;
  virtual int height() override;

  virtual void actionListenerCallback(const String &message) override;

private:
  Trigger *_trigger;
  MidiMessage _input_trigger_message;

  Label _key_label { {}, "Trigger Key" };
  KeyLearner _key;
  TextButton _key_learn { "Set Key" };
  TextButton _key_clear { "Clear" };

  Label _message_label { {}, "Trigger Message" };
  Label _message { {}, "(None)" };
  TextButton _message_learn { "Learn" };
  TextButton _message_clear { "Clear" };

  Label _action_label { {}, "Action" };
  ComboBox _action;

  virtual void layout(Rectangle<int> &area) override;
  void layout_key(Rectangle<int> &area);
  void layout_message(Rectangle<int> &area);
  void layout_action(Rectangle<int> &area);

  virtual void init() override;
  void init_key();
  void init_message();
  void init_action();
  virtual void cancel_cleanup() override;

  virtual bool apply() override;

  void draw_input_message();
};

// If trigger is nullptr we create a new one.
TriggerEditor * open_trigger_editor(Trigger *t);
