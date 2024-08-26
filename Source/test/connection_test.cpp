#include "connection_test.h"

void ConnectionTest::initialise() {
  km = new KeyMaster(dev_mgr, true);
  input = new Input();
  output = new Output();
  input_ptr = input;
  output_ptr = output;
}

void ConnectionTest::runTest() {
  channels_test();
}

void ConnectionTest::channels_test() {
  TestConnection conn(input_ptr, 0, output_ptr, 1);

  beginTest("channels");

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
