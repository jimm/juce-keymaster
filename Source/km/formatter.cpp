#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <JuceHeader.h>
#include "formatter.h"
#include "consts.h"
#include "utils.h"

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

String trigger_message_description(const MidiMessage &msg) {
  if (mm_equal(msg, EMPTY_MESSAGE))
    return "";

  const uint8 *tdata = msg.getRawData();
  uint8 status = tdata[0];

  if (is_system(status))
    return msg.getDescription();

  // Modified version of MidiMessage::getDescription()
  if (msg.isNoteOn())           return "Note on "  + MidiMessage::getMidiNoteName(msg.getNoteNumber(), true, true, 3) + ", Channel " + String(msg.getChannel());
  if (msg.isNoteOff())          return "Note off " + MidiMessage::getMidiNoteName(msg.getNoteNumber(), true, true, 3) + ", Channel " + String(msg.getChannel());
  if (msg.isProgramChange())    return "Program change " + String(msg.getProgramChangeNumber()) + ", Channel " + String(msg.getChannel());
  if (msg.isPitchWheel())       return "Pitch wheel, Channel " + String(msg.getChannel());
  if (msg.isAftertouch())       return "Aftertouch " + MidiMessage::getMidiNoteName(msg.getNoteNumber(), true, true, 3) + ", Channel " + String(msg.getChannel());
  if (msg.isChannelPressure())  return "Channel pressure, Channel " + String(msg.getChannel());
  if (msg.isAllNotesOff())      return "All notes off, Channel " + String(msg.getChannel());
  if (msg.isAllSoundOff())      return "All sound off, Channel " + String(msg.getChannel());
  if (msg.isMetaEvent())        return "Meta event";
  if (msg.isController()) {
    String name (MidiMessage::getControllerName(msg.getControllerNumber()));
    if (name.isEmpty())
      name = String(msg.getControllerNumber());
    return "Controller " + name + ", Channel " + String(msg.getChannel());
  }
  return String::toHexString(msg.getRawData(), msg.getRawDataSize());
}
