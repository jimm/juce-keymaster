#include "../km/consts.h"
#include "../km/connection.h"
#include "../km/patch.h"
#include "patch_table.h"

PatchTableListBoxModel::PatchTableListBoxModel(Patch *p)
  : _patch(p)
{
  addAndMakeVisible(_table);
  _table.setModel(this);
  _table.setColour(ListBox::outlineColourId, Colours::grey);
  _table.setOutlineThickness(1);

  auto &h = _table.getHeader();
  h.addColumn("Input", 1, 75);
  h.addColumn("Chan", 2, 30);
  h.addColumn("Output", 3, 75);
  h.addColumn("Chan", 4, 20);
  h.addColumn("Zone", 5, 30);
  h.addColumn("Xpose", 6, 20);
  h.addColumn("Prog", 7, 60);
  h.addColumn("CC Filts/Maps", 8, 200);
  h.setStretchToFitActive(true);
}

void PatchTableListBoxModel::paintRowBackground(
  Graphics& g, int rowNumber, int _width, int _height, bool rowIsSelected)
{
  auto alternateColour = getLookAndFeel().findColour(ListBox::backgroundColourId)
    .interpolatedWith(getLookAndFeel().findColour(ListBox::textColourId), 0.03f);
  if (rowNumber % 2)
    g.fillAll(alternateColour);
}

void PatchTableListBoxModel::paintCell(
  Graphics& g, int rowNumber, int columnId, int width, int height, bool _rowIsSelected)
{
  if (_patch == nullptr)
    return;

  Connection *c = _patch->connections()[rowNumber];

  g.setColour(getLookAndFeel().findColour(ListBox::textColourId));
  g.setFont(FontOptions(14.0f));

  String str;
  switch (columnId) {
  case 1:                       // input name
    str = c->input_info().name;
    break;
  case 2:                       // input chan
    if (c->input_chan() == CONNECTION_ALL_CHANNELS)
      str = String("All");
    else
      str = String(c->input_chan());
    break;
  case 3:                       // output name
    str = c->output()->getName();
    break;
  case 4:                       // output chan
    if (c->output_chan() == CONNECTION_ALL_CHANNELS)
      str = String("Input");
    else
      str = String(c->output_chan());
    g.drawText(str, 2, 0, width - 4, height, Justification::centredLeft, true);
    break;
  case 5:                       // zone
    if (c->zone_low() != 0 || c->zone_high() != 127)
      str = String::formatted("%d - %d", c->zone_low(), c->zone_high());
    break;
  case 6:                       // xpose
    if (c->xpose() != 0)
      str = String(c->xpose());
    break;
  case 7:                       // prog
    str = program_str(c);
    break;
  case 8:                       // CC filters / maps
    str = controllers_str(c);
    break;
  }

  g.drawText(str, 2, 0, width - 4, height, Justification::centredLeft, true);
  g.setColour(getLookAndFeel().findColour(ListBox::backgroundColourId));
  g.fillRect(width - 1, 0, 1, height);
}

void PatchTableListBoxModel::resized() {
  _table.setBoundsInset(BorderSize<int>(2));
}

String PatchTableListBoxModel::program_str(Connection *c) {
  String str;
  int has_msb = c->program_bank_msb() != UNDEFINED;
  int has_lsb = c->program_bank_lsb() != UNDEFINED;
  int has_bank = has_msb || has_lsb;

  if (has_bank)
    str += '[';

  if (has_msb)
    str += String::formatted("%3d", c->program_bank_msb());

  if (has_bank)
    str += ',';

  if (has_lsb)
    str += String::formatted("%3d", c->program_bank_lsb());
  else
    str += " ";

  if (has_bank)
    str += ']';

  if (c->program_prog() != UNDEFINED)
    str += String::formatted("%d", c->program_prog());
    
  return str;
}

String PatchTableListBoxModel::controllers_str(Connection *c) {
  StringArray arr;

  for (int i = 0; i < 128; ++i) {
    Controller *cc = c->cc_map(i);
    if (cc == nullptr)
      continue;

    String str;
    str += cc->cc_num();

    if (cc->filtered()) {
      str += 'x';
      continue;
    }

    if (cc->cc_num() != cc->translated_cc_num())
      str += String::formatted("->%d", cc->translated_cc_num());

    if (cc->min_in() != 0 || cc->max_in() != 127
        || cc->min_out() != 0 || cc->max_out() != 127)
    {
      str += ' ';
      if (cc->pass_through_0())
        str += 0;
      str += String::formatted(" [%d, %d] -> [%d, %d]",
                               cc->min_in(), cc->max_in(),
                               cc->min_out(), cc->max_out());
      if (cc->pass_through_127())
        str += 127;
    }

    arr.add(str);
  }
  return arr.joinIntoString(", ");
}
