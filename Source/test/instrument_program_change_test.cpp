#include "instrument_program_change_test.h"
#include "../km/instrument_program_change.h"
#include "../km/consts.h"

void InstrumentProgramChangeTest::initialise() {
  km = new KeyMaster(dev_mgr, true);
}

void InstrumentProgramChangeTest::shutdown() {
  delete km;
}

void InstrumentProgramChangeTest::runTest() {
  test_send_all_fields();
  test_send_prog_only();
  test_send_undefined_fields();
}

void InstrumentProgramChangeTest::test_send_all_fields() {
  beginTest("send: MSB + LSB + prog");

  Output *raw_out = new Output();
  Output::Ptr out = raw_out;
  TestMidiOutputListener listener;
  out->add_listener(&listener);

  InstrumentProgramChange ipc;
  ipc.output = out;
  ipc.channel = 3;
  ipc.bank_msb = 5;
  ipc.bank_lsb = 2;
  ipc.prog = 42;
  ipc.send();

  expect(listener.sent.size() == 3);
  expect(listener.sent[0].isController());
  expect(listener.sent[0].getControllerNumber() == CC_BANK_SELECT_MSB);
  expect(listener.sent[0].getControllerValue() == 5);
  expect(listener.sent[1].isController());
  expect(listener.sent[1].getControllerNumber() == CC_BANK_SELECT_LSB);
  expect(listener.sent[1].getControllerValue() == 2);
  expect(listener.sent[2].isProgramChange());
  expect(listener.sent[2].getProgramChangeNumber() == 42);
}

void InstrumentProgramChangeTest::test_send_prog_only() {
  beginTest("send: prog only");

  Output *raw_out = new Output();
  Output::Ptr out = raw_out;
  TestMidiOutputListener listener;
  out->add_listener(&listener);

  InstrumentProgramChange ipc;
  ipc.output = out;
  ipc.channel = 1;
  ipc.prog = 10;
  ipc.send();

  expect(listener.sent.size() == 1);
  expect(listener.sent[0].isProgramChange());
  expect(listener.sent[0].getProgramChangeNumber() == 10);
}

void InstrumentProgramChangeTest::test_send_undefined_fields() {
  beginTest("send: all UNDEFINED sends nothing");

  Output *raw_out = new Output();
  Output::Ptr out = raw_out;
  TestMidiOutputListener listener;
  out->add_listener(&listener);

  InstrumentProgramChange ipc;
  ipc.output = out;
  ipc.channel = 1;
  ipc.send();

  expect(listener.sent.size() == 0);
}
