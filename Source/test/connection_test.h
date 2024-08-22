#pragma once

#ifdef JUCE_DEBUG

#include "../km/keymaster.h"
#include "../km/connection.h"
#include "../km/input.h"
#include "../km/output.h"

Input *input = new Input();
Output *output = new Output();

class TestConnection : public Connection {
public:
  TestConnection(Input::Ptr ip, int ichan, Output::Ptr op, int ochan)
    : Connection(UNDEFINED_ID, ip, 0, op, 1)
    {}

  Array<MidiMessage> sent;

protected:
  void midi_out(MidiMessage *msg) { sent.add(*msg); }
  void midi_out(MidiMessage msg) { sent.add(msg); }
};


class ConnectionTest : public UnitTest {
public:
  ConnectionTest() : UnitTest("Connection Test", "[connection]") {}

  void runTest() override {
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
};

static ConnectionTest test;

#endif
