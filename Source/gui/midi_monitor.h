#pragma once

#include <JuceHeader.h>
#include "../km/midi_listener.h"

class MidiMonitor : public Component, public MidiListener, public AsyncUpdater {
public:
  MidiMonitor();
  virtual ~MidiMonitor();

  int width();
  int height();

  void resized() override;

  virtual void midi_input(const String &name, const MidiMessage &message) override;
  virtual void midi_output(const String &name, const MidiMessage &message) override;

  void handleAsyncUpdate() override;

private:
  struct NameAndMessage {
    String name;
    MidiMessage message;
  };

  TextEditor _input_midi;
  TextEditor _output_midi;
  CriticalSection _midi_monitor_lock;
  Array<NameAndMessage> _input_messages;
  Array<NameAndMessage> _output_messages;

  void init_text_editor(TextEditor &te);

  bool want_message(const MidiMessage &message);
};
