#pragma once

#include <JuceHeader.h>

String note_num_to_name(int num);
int note_name_to_num(String &str); // str may point to an integer string like "64" as well
