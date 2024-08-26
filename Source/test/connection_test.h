#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"
#include "../km/connection.h"
#include "../km/input.h"
#include "../km/output.h"

class TestConnection : public Connection {
public:
  TestConnection(Input::Ptr _in, int ichan, Output::Ptr _out, int ochan)
    : Connection(UNDEFINED_ID, _in, ichan, _out, ochan)
    {}

  Array<MidiMessage> sent;

protected:
  void midi_out(MidiMessage *msg) { sent.add(*msg); }
  void midi_out(MidiMessage msg) { sent.add(msg); }
};


class ConnectionTest : public UnitTest {
public:
  ConnectionTest() : UnitTest("Connection Test", "km") {}

  void initialise() override;
  void runTest() override;

private:
  DeviceManager dev_mgr;
  KeyMaster *km;
  Input *input;
  Output *output;
  Input::Ptr input_ptr;
  Output::Ptr output_ptr;

  void channels_test();
};

static ConnectionTest test;
