#include "test_helper.h"
#include "message_block_test.h"

void MessageBlockTest::runTest() {
  test_from_hex_string();
  test_to_hex_string();
  test_to_editable_hex_string();
}

void MessageBlockTest::test_from_hex_string() {
  beginTest("from hex string");

  MessageBlock m(UNDEFINED_ID, "");
  MidiMessage msg, expected;
  uint8 sysex_data[3] = {0, 1, 2};

  m.from_hex_string("f6");
  expectEquals(m.midi_messages().size(), 1);
  expect(mm_eq(msg = m.midi_messages()[0], expected = MidiMessage(0xf6)));

  m.from_hex_string("b3 12 34");
  expectEquals(m.midi_messages().size(), 1);
  expect(mm_eq(msg = m.midi_messages()[0], expected = MidiMessage(0xb3, 0x12, 0x34)));

  // also tests parsing of editable version (newlines)
  m.from_hex_string("b3 12 34 f6 f0 00 01 02 f7 92 80 ff\n82 80 ff");
  expectEquals(m.midi_messages().size(), 5);
  expect(mm_eq(msg = m.midi_messages()[0], expected = MidiMessage(0xb3, 0x12, 0x34)));
  expect(mm_eq(msg = m.midi_messages()[1], expected = MidiMessage(0xf6)));
  expect(mm_eq(msg = m.midi_messages()[2], expected = MidiMessage::createSysExMessage(sysex_data, 3)));
  expect(mm_eq(msg = m.midi_messages()[3], expected = MidiMessage(0x92, 0x80, 0xff)));
  expect(mm_eq(msg = m.midi_messages()[4], expected = MidiMessage(0x82, 0x80, 0xff)));

  auto result = m.from_hex_string("fff");
  expect(result.failed());
  expect(result.getErrorMessage() == "byte 1 is not between 0 (00) and 255 (ff)");
}

void MessageBlockTest::test_to_hex_string() {
  beginTest("to hex string");
  MessageBlock m(UNDEFINED_ID, "");
  uint8 sysex_data[3] = {0, 1, 2};

  m.midi_messages().add(MidiMessage(0xf6));
  expect(m.to_hex_string() == "f6");

  m.midi_messages().add(MidiMessage(0xb3, 0x12, 0x34));
  expect(m.to_hex_string() == "f6 b3 12 34");

  m.midi_messages().add(MidiMessage::createSysExMessage(sysex_data, 3));
  expect(m.to_hex_string() == "f6 b3 12 34 f0 00 01 02 f7");
}

void MessageBlockTest::test_to_editable_hex_string() {
  beginTest("to editable hex string");
  MessageBlock m(UNDEFINED_ID, "");
  uint8 sysex_data[3] = {0, 1, 2};

  m.midi_messages().add(MidiMessage(0xf6));
  expect(m.to_editable_hex_string() == "f6");

  m.midi_messages().add(MidiMessage(0xb3, 0x12, 0x34));
  expect(m.to_editable_hex_string() == "f6\nb3 12 34");

  m.midi_messages().add(MidiMessage::createSysExMessage(sysex_data, 3));
  expect(m.to_editable_hex_string() == "f6\nb3 12 34\nf0 00 01 02 f7");
}
