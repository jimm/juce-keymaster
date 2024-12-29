#pragma once

#include <JuceHeader.h>
#include "../km/midi_listener.h"

class MidiMonitor : public Component, public MidiListener, public AsyncUpdater {
public:
  MidiMonitor();
  virtual ~MidiMonitor();

  void start();
  void stop();

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

  Label _input_label { {}, "Input" };
  Label _output_label { {}, "Output" };
  TextEditor _input_midi;
  TextEditor _output_midi;
  CriticalSection _midi_monitor_lock;
  Array<NameAndMessage> _input_messages;
  Array<NameAndMessage> _output_messages;
  bool _running;

  void init_text_editor(Label &l, TextEditor &te);

  bool want_message(const MidiMessage &message);
};

class MidiMonitorWindow : public DocumentWindow {
public:
  MidiMonitorWindow(ApplicationProperties &app_properties);
  virtual ~MidiMonitorWindow();

  virtual void closeButtonPressed() override;

  MidiMonitor *midi_monitor() {
    return static_cast<MidiMonitor *>(getContentComponent());
  }

private:
  ApplicationProperties &_app_properties;
};
