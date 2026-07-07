#include "program_change_editor.h"
#include "../km/consts.h"
#include "../km/keymaster.h"

#define INSTRUMENT_WIDTH 250
#define CHANNEL_COMBO_WIDTH 125
#define VALUE_WIDTH 60

#define CONTENT_WIDTH 300
#define CONTENT_HEIGHT (SPACE * 5 + BETWEEN_ROW_SPACE * 4 + LABEL_HEIGHT * 5 + DATA_ROW_HEIGHT * 5)

ProgramChangeEditor * open_program_change_editor(Connection *conn, ProgramChange *pc)
{
  bool is_new = pc == nullptr;
  if (is_new)
    pc = new ProgramChange();

  DialogWindow::LaunchOptions opts;
  opts.dialogTitle = "Program Change";
  opts.dialogBackgroundColour =
    LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
  opts.resizable = false;
  auto editor = new ProgramChangeEditor(conn, pc, is_new);
  opts.content.setOwned(editor);
  auto dialog_win = opts.launchAsync();
  if (dialog_win != nullptr)
    dialog_win->centreWithSize(editor->width(), editor->height());
  return editor;
}

ProgramChangeEditor::ProgramChangeEditor(Connection *conn, ProgramChange *pc, bool is_new)
  : KmEditor(is_new), _conn(conn), _pc(pc)
{
  init();
}

int ProgramChangeEditor::width() {
  return KmEditor::width() + CONTENT_WIDTH;
}

int ProgramChangeEditor::height() {
  return KmEditor::height() + CONTENT_HEIGHT;
}

void ProgramChangeEditor::layout(Rectangle<int> &area) {
  _output_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  _output_instrument.setBounds(area.removeFromTop(DATA_ROW_HEIGHT).removeFromLeft(INSTRUMENT_WIDTH));

  area.removeFromTop(BETWEEN_ROW_SPACE);
  _chan_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  _chan.setBounds(area.removeFromTop(DATA_ROW_HEIGHT).removeFromLeft(CHANNEL_COMBO_WIDTH));

  area.removeFromTop(BETWEEN_ROW_SPACE);
  _msb_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  _msb.setBounds(area.removeFromTop(DATA_ROW_HEIGHT).removeFromLeft(VALUE_WIDTH));

  area.removeFromTop(BETWEEN_ROW_SPACE);
  _lsb_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  _lsb.setBounds(area.removeFromTop(DATA_ROW_HEIGHT).removeFromLeft(VALUE_WIDTH));

  area.removeFromTop(BETWEEN_ROW_SPACE);
  _prog_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  _prog.setBounds(area.removeFromTop(DATA_ROW_HEIGHT).removeFromLeft(VALUE_WIDTH));

  KmEditor::layout(area);
}

void ProgramChangeEditor::init() {
  _output_instrument.addItem("None (no output)", UNSELECTED);
  int i = 1;
  for (auto outp : KeyMaster_instance()->device_manager().outputs()) {
    _output_instrument.addItem(outp->name(), i);
    if (outp == _pc->output)
      _output_instrument.setSelectedId(i);
    ++i;
  }
  if (_output_instrument.getSelectedId() == 0)
    _output_instrument.setSelectedId(UNSELECTED);

  _chan.addItem("None", CONNECTION_ALL_CHANNELS);
  for (i = 1; i <= MIDI_CHANNELS; ++i)
    _chan.addItem(String(i), i);
  if (_pc->chan == CONNECTION_ALL_CHANNELS)
    _chan.setSelectedId(CONNECTION_ALL_CHANNELS);
  else
    _chan.setSelectedId(_pc->chan + 1);

  init_text_editor(_msb, _pc->bank_msb == UNDEFINED ? "" : String(_pc->bank_msb));
  init_text_editor(_lsb, _pc->bank_lsb == UNDEFINED ? "" : String(_pc->bank_lsb));
  init_text_editor(_prog, _pc->prog == UNDEFINED ? "" : String(_pc->prog));

  addAndMakeVisible(_output_label);
  addAndMakeVisible(_output_instrument);
  addAndMakeVisible(_chan_label);
  addAndMakeVisible(_chan);
  addAndMakeVisible(_msb_label);
  addAndMakeVisible(_msb);
  addAndMakeVisible(_lsb_label);
  addAndMakeVisible(_lsb);
  addAndMakeVisible(_prog_label);
  addAndMakeVisible(_prog);

  KmEditor::init();
}

void ProgramChangeEditor::init_text_editor(TextEditor &te, String initial_contents) {
  te.setSelectAllWhenFocused(true);
  te.setEscapeAndReturnKeysConsumed(false);
  te.setColour(TextEditor::outlineColourId, findColour(ComboBox::outlineColourId));
  te.setText(initial_contents);
  te.setCaretPosition(initial_contents.length());
  addAndMakeVisible(te);
}

void ProgramChangeEditor::cancel_cleanup() {
  if (_is_new)
    delete _pc;
}

bool ProgramChangeEditor::apply() {
  DeviceManager &dm = KeyMaster_instance()->device_manager();
  int id = _output_instrument.getSelectedId();
  Output::Ptr output = id == UNSELECTED ? nullptr : dm.outputs()[id-1];

  int chan = _chan.getSelectedId();
  if (chan != CONNECTION_ALL_CHANNELS)
    --chan;

  auto text = _msb.getText();
  int msb = text.isEmpty() ? UNDEFINED : text.getIntValue();
  if (msb != UNDEFINED && (msb < 0 || msb > 127))
    add_error_message("Bank MSB value must be 0-127");

  text = _lsb.getText();
  int lsb = text.isEmpty() ? UNDEFINED : text.getIntValue();
  if (lsb != UNDEFINED && (lsb < 0 || lsb > 127))
    add_error_message("Bank LSB value must be 0-127");

  text = _prog.getText();
  int prog = text.isEmpty() ? UNDEFINED : text.getIntValue();
  if (prog != UNDEFINED && (prog < 0 || prog > 127))
    add_error_message("Program value must be 0-127");

  if (has_errors()) {
    display_errors("Program Change");
    return false;
  }

  _pc->output = output;
  _pc->chan = chan;
  _pc->bank_msb = msb;
  _pc->bank_lsb = lsb;
  _pc->prog = prog;

  if (_is_new) {
    _conn->add_program_change(_pc);
    _is_new = false;
  } else {
    _conn->program_changes_changed();
  }

  sendActionMessage("update:table-list-box");
  return true;
}
