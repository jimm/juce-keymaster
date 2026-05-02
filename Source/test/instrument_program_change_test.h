#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"
#include "../km/midi_listener.h"

class TestMidiOutputListener : public MidiListener {
public:
  Array<MidiMessage> sent;

  void midi_output(const String &, const MidiMessage &msg) override {
    sent.add(msg);
  }
};

class InstrumentProgramChangeTest : public UnitTest {
public:
  InstrumentProgramChangeTest()
    : UnitTest("Instrument Program Change Test", "km:ipc") {}

  void initialise() override;
  void shutdown() override;
  void runTest() override;

private:
  DeviceManager dev_mgr;
  KeyMaster *km;

  void test_send_all_fields();
  void test_send_prog_only();
  void test_send_undefined_fields();
};

static InstrumentProgramChangeTest instrument_program_change_test;
