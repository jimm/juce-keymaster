#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <JuceHeader.h>
#include "formatter.h"
#include "consts.h"

static const int NOTE_OFFSETS[] = {
  9, 11, 0, 2, 4, 5, 7
};

String note_num_to_name(int num) {
  return MidiMessage::getMidiNoteName(num, true, true, 4);
}

// str may point to an integer string like "64" as well
int note_name_to_num(String &str) {
  char ch = (char)str[0];

  if (isdigit(ch))
    return str.getIntValue();

  ch = tolower(ch);
  if (ch < 'a' || ch > 'g')
    return 0;

  int from_c = NOTE_OFFSETS[ch - 'a'];
  int num_start = 1;
  int accidental = 0;
  switch (tolower((char)str[1])) {
  case 's': case '#':
    accidental = 1;
    ++num_start;
    break;
  case 'f': case 'b':
    accidental = -1;
    ++num_start;
    break;
  }

  int octave = (str.substring(num_start).getIntValue() + 1) * 12;
  return octave + from_c + accidental;
}

String format_program(int bank_msb, int bank_lsb, int prog) {
  String str;
  int has_msb = bank_msb != UNDEFINED;
  int has_lsb = bank_lsb != UNDEFINED;
  int has_bank = has_msb || has_lsb;

  str += String::formatted(" %c", has_bank ? '[' : ' ');

  if (has_msb)
    str += String::formatted("%3d", bank_msb);
  else
    str += "   ";

  str += String::formatted("%c ", has_bank ? ',' : ' ');

  if (has_lsb)
    str += String::formatted("%3d", bank_lsb);
  else
    str += "   ";

  str += String::formatted("%c ", has_bank ? ']' : ' ');

  if (prog != UNDEFINED)
    str += String::formatted(" %3d", prog);
  else
    str += "    ";

  return str;
}
