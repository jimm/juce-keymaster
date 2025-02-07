#include "test_helper.h"
#include "device_manager_test.h"
#include "../km/keymaster.h"

void DeviceManagerTest::runTest() {
  test_find_and_create();
  test_different_device_identifiers();
}

void DeviceManagerTest::test_find_and_create() {
  clear_dev_mgr();
  beginTest("find and create inputs");

  // start empty
  expect(dev_mgr.inputs().size() == 0);
  expect(dev_mgr.find_input("in1") == nullptr);
  expect(dev_mgr.outputs().size() == 0);
  expect(dev_mgr.find_output("out1") == nullptr);

  // Create an input and look it up
  dev_mgr.find_or_create_input("in1", "input 1");
  auto inp = dev_mgr.find_input("in1");
  expect(inp != nullptr);
  expect(inp->identifier() == "in1");
  expect(inp->name() == "input 1");

  // Create an output and look it up
  dev_mgr.find_or_create_output("out1", "output 1");
  auto outp = dev_mgr.find_output("out1");
  expect(outp != nullptr);
  expect(outp->identifier() == "out1");
  expect(outp->name() == "output 1");

  // Make sure nothing is horribly wrong
  expect(dev_mgr.find_input("in1") == inp);
  expect(dev_mgr.find_output("out1") == outp);
}

void DeviceManagerTest::test_different_device_identifiers() {
  clear_dev_mgr();
  beginTest("uses instrument name if identifier not found");

  // Create some instruments under one "local" set of identifiers. We will
  // look them up with different identifiers but the same name, to test the
  // case where an input file has been brought to a different OS.
  auto local_inp = dev_mgr.find_or_create_input(MidiDeviceInfo("first input", "local-i1"));
  auto local_outp = dev_mgr.find_or_create_output(MidiDeviceInfo("first output", "local-o1"));

  expect(dev_mgr.find_input("local-i1") != nullptr);
  expect(dev_mgr.find_output("local-o1") != nullptr);

  // ================ inputs ================

  // Make sure we have the local id, not the foreign one yet
  expect(dev_mgr.find_input("foreign-i1") == nullptr); // make sure we don't have the foreign id

  // Make sure we find input with same name even if ids are different
  auto inp = dev_mgr.find_or_create_input(MidiDeviceInfo("first input", "foreign-i1"));
  expect(inp == local_inp);
  expect(dev_mgr.find_input("local-i1") == inp);
  expect(dev_mgr.find_input("foreign-i1") == nullptr);

  // ================ outputs ================

  // Make sure we have the local id, not the foreign one yet
  expect(dev_mgr.find_output("foreign-o1") == nullptr);

  // Make sure we find output with same name even if ids are different
  auto outp = dev_mgr.find_or_create_output(MidiDeviceInfo("first output", "foreign-o1"));
  expect(outp == local_outp);
  expect(dev_mgr.find_output("local-o1") == outp);
  expect(dev_mgr.find_output("foreign-o1") == nullptr);
}

void DeviceManagerTest::clear_dev_mgr() {
  dev_mgr.clear();
}
