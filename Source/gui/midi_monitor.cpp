#include <JuceHeader.h>
#include "../km/keymaster.h"
#include "midi_monitor.h"

#define SPACE 12
#define INITIAL_WIDTH 800
#define INITIAL_HEIGHT 600

MidiMonitor::MidiMonitor() : _running(false) {
  init_text_editor(_input_midi);
  init_text_editor(_output_midi);
  setSize(width(), height());
}

MidiMonitor::~MidiMonitor() {
  stop();
}

void MidiMonitor::start() {
  if (_running)
    return;

  _input_midi.setText("");
  _output_midi.setText("");

  auto device_manager = KeyMaster_instance()->device_manager();

  for (auto inp : device_manager.inputs())
    inp->add_listener(this);
  for (auto outp : device_manager.outputs())
    outp->add_listener(this);

  _running = true;
}

void MidiMonitor::stop() {
  if (!_running)
    return;

  auto km = KeyMaster_instance();
  if (km == nullptr)            // might be shutting down
    return;
  auto device_manager = km->device_manager();

  for (auto inp : device_manager.inputs())
    inp->remove_listener(this);
  for (auto outp : device_manager.outputs())
    outp->remove_listener(this);

  _running = false;
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

// ================================================================

MidiMonitorWindow::MidiMonitorWindow()
  : DocumentWindow(
      "MIDI Monitor",
      LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
      TitleBarButtons::allButtons,
      true
    )
{
  auto contents = new MidiMonitor();
  setContentOwned(contents, false);

  // TODO load from app settings
  auto display_area = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.reduced(20);
  Rectangle<int> area(
    (display_area.getWidth() - contents->width()) / 2,
    (display_area.getHeight() - contents->height()) / 3,
    contents->width(),
    contents->height()
  );
  setBounds(area);

  setResizable(true, true);
  setUsingNativeTitleBar(true);

  contents->start();
}

// We don't close the window, we stop monitoring and hide it.
void MidiMonitorWindow::closeButtonPressed() {
  midi_monitor()->stop();
  setVisible(false);
}
