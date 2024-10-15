#include "understanding_test.h"
#include "../km/consts.h"
#include "../km/trigger.h"

// These tests help me understand how JUCE operates.

void UnderstandingTest::runTest() {
  // if message has one status and one data byte, can I construct it this way?
  // If not, this will blow up.
  auto msg = MidiMessage(PROGRAM_CHANGE, 42);

  // How do we determine if a KeyPress has been defined?
  expect(!KeyPress().isValid());
  auto trigger = Trigger(UNDEFINED, "trig", TriggerAction::NEXT_SONG, nullptr);
  expect(trigger.has_trigger_key_press() == false);

  // What does F1 look like?
  auto f1 = KeyPress(KeyPress::F1Key);
  expect(f1.getTextDescriptionWithIcons() == "F1");
  // DBG(f1.getKeyCode());
}
