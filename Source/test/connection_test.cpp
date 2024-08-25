#include "connection_test.h"

Input *input = new Input();
Output *output = new Output();

void ConnectionTest::runTest() {
  DeviceManager dev_mgr;
  KeyMaster km(dev_mgr, true);
  Input::Ptr input_ptr(input);
  Output::Ptr output_ptr(output);
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
