#include <JuceHeader.h>
#include "../km/keymaster.h"
#include "midi_monitor.h"

#define SPACE 12
#define INITIAL_WIDTH 800
#define INITIAL_HEIGHT 600

MidiMonitor::MidiMonitor() {
  init_text_editor(_input_midi);
  init_text_editor(_output_midi);
  setSize(width(), height());

  for (auto inp : KeyMaster_instance()->device_manager().inputs())
    inp->add_listener(this);

  for (auto outp : KeyMaster_instance()->device_manager().outputs())
    outp->add_listener(this);
}

MidiMonitor::~MidiMonitor() {

  for (auto inp : KeyMaster_instance()->device_manager().inputs())
    inp->remove_listener(this);

  for (auto outp : KeyMaster_instance()->device_manager().outputs())
    outp->remove_listener(this);
}

int MidiMonitor::width() { return INITIAL_WIDTH; }

int MidiMonitor::height() { return INITIAL_HEIGHT; }

void MidiMonitor::resized() {
  auto area = getLocalBounds();
  area.reduce(SPACE, SPACE);

  int width = (area.getWidth() - SPACE) / 2;
  _input_midi.setBounds(area.removeFromLeft(width));
  _output_midi.setBounds(area.removeFromRight(width));
}

void MidiMonitor::init_text_editor(TextEditor &te) {
  te.setMultiLine(true);
  te.setReturnKeyStartsNewLine(false);
  te.setReadOnly(true);
  te.setScrollbarsShown(true);
  te.setCaretVisible(false);
  te.setPopupMenuEnabled(false);
  addAndMakeVisible(te);
}

void MidiMonitor::midi_input(const String &name, const MidiMessage &message) {
  if (!want_message(message))
    return;

  const ScopedLock sl(_midi_monitor_lock);
  _input_messages.add(NameAndMessage(name, message));
  triggerAsyncUpdate();
}

void MidiMonitor::midi_output(const String &name, const MidiMessage &message) {
  if (!want_message(message))
    return;

  const ScopedLock sl(_midi_monitor_lock);
  _output_messages.add(NameAndMessage(name, message));
  triggerAsyncUpdate();
}

void MidiMonitor::handleAsyncUpdate() {
  DBG("updating monitor");      // DEBUG

  Array<NameAndMessage> input_msgs;
  Array<NameAndMessage> output_msgs;

  {
    const ScopedLock sl(_midi_monitor_lock);
    input_msgs = _input_messages;
    output_msgs = _output_messages;
  }

  String text;
  for (auto &n_and_m : input_msgs)
    text << n_and_m.name << "\t" << n_and_m.message.getDescription() << "\n";
  _input_midi.insertTextAtCaret(text);

  text = "";
  for (auto &n_and_m : output_msgs)
    text << n_and_m.name << "\t" << n_and_m.message.getDescription() << "\n";
  _output_midi.insertTextAtCaret(text);
}

bool MidiMonitor::want_message(const MidiMessage &message) {
  return !message.isMidiClock() && !message.isActiveSense();
}
