#pragma once

#include <JuceHeader.h>
#include "../km/keymaster.h"

class MessageBlockTest : public UnitTest {
public:
  MessageBlockTest() : UnitTest("Message Block Test", "km:message") {}

  void runTest() override;

private:
  void test_from_hex_string();
  void test_to_hex_string();
  void test_to_editable_hex_string();
};

static MessageBlockTest message_block_test;
