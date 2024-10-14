#include "test_helper.h"
#include "controller_test.h"

void ControllerTest::runTest() {
  Controller cc(UNDEFINED_ID, 7);
  MidiMessage msg, expected;

  beginTest("out chan");
  expect(mm_eq(msg = cc.process(MidiMessage::controllerEvent(JCH(0), 7, 127), 3),
               expected = MidiMessage::controllerEvent(JCH(3), 7, 127)));
               

  beginTest("filter");
  cc.set_filtered(true);
  expect(mm_eq(msg = cc.process(MidiMessage::controllerEvent(JCH(0), 7, 127), 0),
               expected = EMPTY_MESSAGE));

  beginTest("map");
  cc.set_filtered(false);
  cc.set_translated_cc_num(10);
  expect(mm_eq(msg = cc.process(MidiMessage::controllerEvent(JCH(0), 7, 127), 0),
               expected = MidiMessage::controllerEvent(JCH(0), 10, 127)));

  beginTest("limit");
  cc.set_translated_cc_num(7);
  cc.set_range(false, true, 1, 120, 1, 120);

  expect(mm_eq(msg = cc.process(MidiMessage::controllerEvent(JCH(0), 7, 0), 0),
               expected = EMPTY_MESSAGE));
  expectEquals(cc.process(MidiMessage::controllerEvent(JCH(0), 7, 1), 0).getControllerValue(), 1);
  expectEquals(cc.process(MidiMessage::controllerEvent(JCH(0), 7, 64), 0).getControllerValue(), 64);
  expectEquals(cc.process(MidiMessage::controllerEvent(JCH(0), 7, 120), 0).getControllerValue(), 120);
  expect(mm_eq(msg = cc.process(MidiMessage::controllerEvent(JCH(0), 7, 121), 0),
               expected = EMPTY_MESSAGE));
  expectEquals(cc.process(MidiMessage::controllerEvent(JCH(0), 7, 127), 0).getControllerValue(), 127);

  beginTest("map range");
  cc.set_range(true, false, 1, 100, 40, 50);

  // 0 pass
  expectEquals(cc.process(MidiMessage::controllerEvent(JCH(0), 7, 0), 0).getControllerValue(), 0);
  // 127 no pass
  expect(mm_eq(msg = cc.process(MidiMessage::controllerEvent(JCH(0), 7, 127), 0),
               expected = EMPTY_MESSAGE));

  expectEquals(cc.process(MidiMessage::controllerEvent(JCH(0), 7, 1), 0).getControllerValue(), 40);
  expectEquals(cc.process(MidiMessage::controllerEvent(JCH(0), 7, 100), 0).getControllerValue(), 50);
  expectEquals(cc.process(MidiMessage::controllerEvent(JCH(0), 7, 50), 0).getControllerValue(), 45);
}
