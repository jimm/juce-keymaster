#pragma once

#include <JuceHeader.h>
#include "../km/message_block.h"
#include "../km/midi_message_learner.h"
#include "km_editor.h"

class MessageEditor
  : public KmEditor, public TextEditor::Listener, public MidiMessageLearner, public ActionListener
{
public:
  MessageEditor(MessageBlock *m, bool is_new);
  virtual ~MessageEditor();

  virtual int width() override;
  virtual int height() override;

private:
  MessageBlock *_message;

  Label _name_label { {}, "Name" };
  TextEditor _name;

  Label _bytes_label { {}, "Message Bytes" };
  TextEditor _bytes;
  TextButton _bytes_learn { "Learn" };
  TextButton _bytes_clear { "Clear" };

  virtual void layout(Rectangle<int> &area) override;
  void layout_name(Rectangle<int> &area);
  void layout_bytes(Rectangle<int> &area);

  virtual void init() override;
  virtual void cancel_cleanup() override;

  virtual bool apply() override;

  virtual void learn_midi_message(const MidiMessage &message) override;

  virtual void actionListenerCallback(const String &message) override;
  virtual void textEditorTextChanged(TextEditor &) override;
};

// If message is nullptr we create a new one.
MessageEditor * open_message_editor(MessageBlock *s);
