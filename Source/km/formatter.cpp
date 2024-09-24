#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <JuceHeader.h>
#include "formatter.h"
#include "consts.h"

static const int NOTE_OFFSETS[] = {
  9, 11, 0, 2, 4, 5, 7
};

// str may point to an integer string like "64" as well
int note_name_to_num(String str) {
  char ch = (char)str[0];

  if (isdigit(ch))
    return str.getIntValue();

  ch = char(tolower(ch));
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
