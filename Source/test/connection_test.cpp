#include "connection_test.h"
#include "test_helper.h"

// Retrieve the nth sent MIDI message
#define SENT(n) conn.sent[n]

void ConnectionTest::initialise() {
  km = new KeyMaster(dev_mgr, true);
  input = new Input();
  output = new Output();
  input_ptr = input;
  output_ptr = output;
}

void ConnectionTest::shutdown() {
  output_ptr = nullptr;
  input_ptr = nullptr;
  delete km;
}

void ConnectionTest::runTest() {
  test_channels_test();
  test_pc_test();
  test_filter_and_modify_test();
  test_editing_when_not_running();
  test_editing_when_running();
}

void ConnectionTest::test_channels_test() {
  TestConnection conn(input_ptr, 0, output_ptr, 1);

  beginTest("start pc channel");

  conn.set_input_chan(CONNECTION_ALL_CHANNELS);
  conn.set_output_chan(CONNECTION_ALL_CHANNELS);
  expect(conn.program_change_send_channel() == CONNECTION_ALL_CHANNELS);

  conn.set_input_chan(3);
  conn.set_output_chan(CONNECTION_ALL_CHANNELS);
  expect(conn.program_change_send_channel() == 3);

  conn.set_input_chan(CONNECTION_ALL_CHANNELS);
  conn.set_output_chan(5);
  expect(conn.program_change_send_channel() == 5);

  conn.set_input_chan(3);
  conn.set_output_chan(4);
  expect(conn.program_change_send_channel() == 4);
}

void ConnectionTest::test_pc_test() {
  TestConnection conn(input_ptr, 0, output_ptr, 1);

  beginTest("start sends pc");

  conn.set_program_prog(123);
  conn.start();
  expect(conn.program_change_send_channel() == 1);
  expect_sent_count(1, conn);
  expect(mm_eq(SENT(0), MidiMessage::programChange(JCH(1), 123)));
}

void ConnectionTest::test_filter_and_modify_test() {
  TestConnection conn(input_ptr, 0, output_ptr, 0);

  beginTest("filter and modify");

  conn.start();

  // filter other input chan
  conn.reset();
  conn.midi_in(input_ptr, MidiMessage::noteOn(JCH(3), 64, (uint8)127));
  expect_sent_count(0, conn);

  // allow all chans
  conn.reset();
  conn.set_input_chan(CONNECTION_ALL_CHANNELS);
  conn.midi_in(input_ptr, MidiMessage::noteOn(JCH(3), 64, (uint8)127));
  expect_sent_count(1, conn);
  expect(mm_eq(SENT(0), MidiMessage::noteOn(JCH(0), 64, (uint8)127))); /* mutated to output chan */

  // allow all chans in and out
  conn.reset();
  conn.set_input_chan(CONNECTION_ALL_CHANNELS);
  conn.set_output_chan(CONNECTION_ALL_CHANNELS);
  conn.midi_in(input_ptr, MidiMessage::noteOn(JCH(3), 64, (uint8)127));
  expect_sent_count(1, conn);
  expect(mm_eq(SENT(0), MidiMessage::noteOn(JCH(3), 64, (uint8)127))); /* out chan not changed */

  // all chans filter controller
  conn.reset();
  conn.set_input_chan(CONNECTION_ALL_CHANNELS);
  conn.set_output_chan(CONNECTION_ALL_CHANNELS);
  conn.set_controller(new Controller(UNDEFINED_ID, 64));
  conn.cc_map(64)->set_filtered(true);
  conn.midi_in(input_ptr, MidiMessage(CONTROLLER + 3, 64, 127));
  expect_sent_count(0, conn);

  // all chans process controller
  conn.reset();
  conn.set_input_chan(3);
  conn.set_output_chan(3);
  conn.set_controller(new Controller(UNDEFINED_ID, 64));
  conn.cc_map(64)->set_range(false, false, 1, 127, 1, 126);
  conn.midi_in(input_ptr, MidiMessage(CONTROLLER + 3, 64, 127));
  expect_sent_count(1, conn);
  expect(mm_eq(SENT(0), MidiMessage(CONTROLLER + 3, 64, 126))); /* out value clamped */

  // !xpose
  conn.reset();

  conn.midi_in(input_ptr, MidiMessage::noteOn(JCH(0), 64, (uint8)127));
  conn.set_xpose(12);
  conn.midi_in(input_ptr, MidiMessage::noteOn(JCH(0), 64, (uint8)127));
  conn.set_xpose(-12);
  conn.midi_in(input_ptr, MidiMessage::noteOn(JCH(0), 64, (uint8)127));

  expect_sent_count(3, conn);
  expect(mm_eq(SENT(0), MidiMessage::noteOn(JCH(0), 64, (uint8)127)));
  expect(mm_eq(SENT(1), MidiMessage::noteOn(JCH(0), 64+12, (uint8)127)));
  expect(mm_eq(SENT(2), MidiMessage::noteOn(JCH(0), 64-12, (uint8)127)));


  // xpose out of range filters out note
  conn.reset();

  conn.midi_in(input_ptr, MidiMessage::noteOn(JCH(0), 64, (uint8)127));
  conn.set_xpose(128);
  conn.midi_in(input_ptr, MidiMessage::noteOn(JCH(0), 64, (uint8)127));

  expect_sent_count(1, conn);
  expect(mm_eq(SENT(0), MidiMessage::noteOn(JCH(0), 64, (uint8)127)));


  Curve *exp = new Curve(UNDEFINED_ID, "Exponential", "exp");
  exp->curve[64] = 32;
  km->add_curve(exp);

  Curve *invexp = new Curve(UNDEFINED_ID, "Inverse Exponential", "-exp");
  invexp->curve[64] = 84;
  km->add_curve(invexp);

  conn.reset();

  conn.midi_in(input_ptr, MidiMessage::noteOn(JCH(0), 64, (uint8)64));
  conn.set_velocity_curve(km->curve_with_name("Exponential"));
  conn.midi_in(input_ptr, MidiMessage::noteOn(JCH(0), 64, (uint8)64));
  conn.set_velocity_curve(km->curve_with_name("Inverse Exponential"));
  conn.midi_in(input_ptr, MidiMessage::noteOn(JCH(0), 64, (uint8)64));
  conn.midi_in(input_ptr, MidiMessage::noteOff(JCH(0), 64, (uint8)64));

  expect_sent_count(4, conn);
  expect(mm_eq(SENT(0), MidiMessage::noteOn(JCH(0),  64, (uint8)64)));
  expect(SENT(1).getVelocity() == (uint8)32); // exponential
  expect(SENT(2).getVelocity() == (uint8)84); // inverse exponential
  expect(SENT(3).getVelocity() == (uint8)84);

  // !zone
  conn.reset();

  conn.set_zone_low(0);
  conn.set_zone_high(64);
  conn.midi_in(input_ptr, MidiMessage::noteOn(JCH(0), 48, (uint8)127));
  conn.midi_in(input_ptr, MidiMessage::noteOff(JCH(0), 48, (uint8)127));
  conn.midi_in(input_ptr, MidiMessage::noteOn(JCH(0), 76, (uint8)127));
  conn.midi_in(input_ptr, MidiMessage::noteOff(JCH(0), 76, (uint8)127));

  expect_sent_count(2, conn);
  expect(mm_eq(SENT(0), MidiMessage::noteOn(JCH(0), 48, (uint8)127)));
  expect(mm_eq(SENT(1), MidiMessage::noteOff(JCH(0), 48, (uint8)127)));


  // zone poly pressure
  conn.reset();

  conn.set_zone_low(0);
  conn.set_zone_high(64);
  conn.midi_in(input_ptr, MidiMessage::aftertouchChange(JCH(0), 48, 127));
  conn.midi_in(input_ptr, MidiMessage::aftertouchChange(JCH(0), 76, 127));

  expect_sent_count(1, conn);
  expect(mm_eq(SENT(0), MidiMessage::aftertouchChange(JCH(0), 48, 127)));


  // cc processed
  conn.reset();
  conn.set_controller(new Controller(UNDEFINED_ID, 7));
  conn.cc_map(7)->set_filtered(true);
  conn.midi_in(input_ptr, MidiMessage(CONTROLLER, 7, 127));

  expect_sent_count(0, conn);


  // message filter
  conn.reset();
  MessageFilter &mf = conn.message_filter();
  Array<MidiMessage> messages;

  // by default, MessageFilters filters out sysex messages

  messages.add(MidiMessage::noteOn(JCH(0), 64, (uint8)127));
  messages.add(MidiMessage::midiStart());
  uint8 sysex_bytes[] = { 1, 2, 3, CLOCK, 4 };
  messages.add(MidiMessage::createSysExMessage(sysex_bytes, 6));
  messages.add(MidiMessage::controllerEvent(JCH(0), 7, 127)); // volume CC
  messages.add(MidiMessage::controllerEvent(JCH(0), CC_BANK_SELECT_MSB, 127)); // bank MSB
  messages.add(MidiMessage::programChange(JCH(0), 2));
  messages.add(MidiMessage::aftertouchChange(JCH(0), 64, 0));
  messages.add(MidiMessage::channelPressureChange(JCH(0), 64));
  messages.add(MidiMessage::noteOff(JCH(0), 64, (uint8)127));
  messages.add(MidiMessage(SYSTEM_RESET));

  // filter sysex, but realtime bytes inside will be sent
  expect(mf.sysex() == false); // check default value

  for (auto msg : messages)
    conn.midi_in(input_ptr, msg);

  expect_sent_count(messages.size(), conn); // +1 for clock, -1 for filtered sysex
  expect(mm_eq(SENT(0), MidiMessage::noteOn(JCH(0), 64, (uint8)127)));
  expect(mm_eq(SENT(2), MidiMessage(CLOCK)));
  expect(mm_eq(SENT(conn.sent.size() - 1), messages.getLast()));
  
  // pass through sysex
  mf.set_sysex(true);

  conn.sent.clear();
  for (auto msg : messages)
    conn.midi_in(input_ptr, msg);

  // we expect same number because clock is inside sysex, and whole sysex
  // including clock is sent as one message
  expect_sent_count(messages.size(), conn);
  expect(mm_eq(SENT(0), MidiMessage::noteOn(JCH(0), 64, (uint8)127)));
  expect(SENT(2).isSysEx());
  expect(mm_eq(SENT(conn.sent.size() - 1), messages.getLast()));
  
  // filter note on and off
  mf.set_note(false);

  conn.sent.clear();
  for (auto msg : messages)
    conn.midi_in(input_ptr, msg);

  expect_sent_count(messages.size() - 2, conn);
  expect(mm_eq(SENT(0), MidiMessage::midiStart()));
  expect(mm_eq(SENT(conn.sent.size() - 2), MidiMessage::channelPressureChange(JCH(0), 64)));
}

void ConnectionTest::test_editing_when_not_running() {
  TestConnection conn(input_ptr, 0, output_ptr, 1);

  beginTest("editing when not running");

  expect(conn.xpose() == 0);
  expect(!conn.is_running());
  conn.begin_changes();
  conn.set_xpose(12);
  conn.end_changes();

  expect(!conn.is_running());
  expect(conn.xpose() == 12);
  expect(conn.input() == input_ptr);
  expect(conn.output() == output_ptr);
}

void ConnectionTest::test_editing_when_running() {
  TestConnection conn(input_ptr, 0, output_ptr, 1);
  Input::Ptr in_old_ptr = input_ptr;
  Input *in_new = new Input();
  Input::Ptr in_new_ptr = in_new;

  beginTest("editing when running and input changes");

  conn.start();

  expect(conn.is_running());
  conn.begin_changes();
  expect(!conn.is_running());

  conn.set_xpose(12);
  conn.set_input(in_new_ptr);

  conn.end_changes();
  expect(conn.is_running());

  expect(conn.xpose() == 12);
  expect(conn.input() == in_new_ptr);
  expect(conn.output() == output_ptr); // unchanged

  conn.stop();
}

void ConnectionTest::expect_sent_count(int expected, TestConnection &conn) {
  int num_sent = conn.sent.size();
  String s("sent messages: expected ");
  s << expected << ", sent " << num_sent;

  expect(expected == num_sent, s);
}
