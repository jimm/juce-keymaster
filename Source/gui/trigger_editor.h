#pragma once

#include <JuceHeader.h>
#include "../km/trigger.h"
#include "../km/midi_message_learner.h"
#include "km_editor.h"
#include "notifying_combo_box.h"

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
  MidiMessage _input_trigger_message;
  Trigger *_trigger;
  Input::Ptr _learning_input;

  Label _key_label { {}, "Trigger Key" };
  KeyLearner _key;
  TextButton _key_learn { "Set Key" };
  TextButton _key_erase { "Remove Key" };

  Label _input_and_message_label { {}, "Trigger Input and Message" };
  Label _input_label { {}, "Input" };
  NotifyingComboBox _input;
  Label _message_label { {}, "MIDI" };
  Label _message { {}, "(None)" };
  TextButton _message_learn { "Learn" };

  Label _action_label { {}, "Action" };
  ComboBox _action;

  virtual void layout(Rectangle<int> &area) override;
  void layout_key(Rectangle<int> &area);
  void layout_input_and_message(Rectangle<int> &area);
  void layout_action(Rectangle<int> &area);

  virtual void init() override;
  void init_key();
  void init_input_and_message();
  void init_action();
  virtual void cancel_cleanup() override;

  virtual bool apply() override;

  Input::Ptr selected_input();
  void draw_input_message();
  virtual void learn_midi_message(const MidiMessage &message) override;
};

// If trigger is nullptr we create a new one.
TriggerEditor * open_trigger_editor(Trigger *t);
