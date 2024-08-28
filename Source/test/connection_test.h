#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"
#include "../km/connection.h"
#include "../km/input.h"
#include "../km/output.h"

class TestConnection : public Connection {
public:
  TestConnection(Input::Ptr _in, int ichan, Output::Ptr _out, int ochan)
    : Connection(UNDEFINED_ID, _in, ichan, _out, ochan),
      _orig_ichan(ichan),
      _orig_ochan(ochan)
    {}

  void reset() {
    set_input_chan(_orig_ichan);
    set_output_chan(_orig_ochan);
    set_xpose(0);
    set_velocity_curve(nullptr);
    set_program_bank_msb(UNDEFINED);
    set_program_bank_lsb(UNDEFINED);
    set_program_prog(UNDEFINED);
    set_zone_low(0);
    set_zone_high(127);
    for (int i = 0; i < 128; ++i)
      set_cc_map(i, nullptr);

    sent.clear();
  }

  Array<MidiMessage> sent;

  void output_sent_messages() { // for debugging when tests fail
    for (auto msg : sent) DBG(msg.getDescription());
  }

protected:
  int _orig_ichan;
  int _orig_ochan;

  virtual void midi_out(MidiMessage *msg) override { sent.add(*msg); }
  virtual void midi_out(MidiMessage msg) override { sent.add(msg); }
};


class ConnectionTest : public UnitTest {
public:
  ConnectionTest() : UnitTest("Connection Test", "km") {}

  void initialise() override;
  void shutdown() override;
  void runTest() override;

private:
  DeviceManager dev_mgr;
  KeyMaster *km;
  Input *input;
  Output *output;
  Input::Ptr input_ptr;
  Output::Ptr output_ptr;

  void test_channels_test();
  void test_pc_test();
  void test_filter_and_modify_test();
  void test_editing_when_not_running();
  void test_editing_when_running();

  void expect_sent_count(int expected, TestConnection &conn);
};

static ConnectionTest connection_test;
