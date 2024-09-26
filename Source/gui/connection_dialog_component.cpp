#include "connection_dialog_component.h"
#include "../km/consts.h"
#include "../km/keymaster.h"
#include "../km/patch.h"
#include "../km/connection.h"
#include "../km/editor.h"
#include "../km/formatter.h"

#define UNSELECTED (-1)

#define SPACE 12
#define BETWEEN_ROW_SPACE 20
#define LABEL_HEIGHT 16
#define BUTTON_HEIGHT 35
#define BUTTON_WIDTH 80
#define DATA_ROW_HEIGHT 20
#define INSTRUMENT_WIDTH 250
#define CHANNEL_COMBO_WIDTH 125
#define PROG_LABEL_WIDTH 35
#define PROG_WIDTH 40
#define ZONE_WIDTH 40
#define ZONE_BETWEEN_WIDTH 16
#define XPOSE_COLUMN_WIDTH 100
#define XPOSE_FIELD_WIDTH 40
#define VCURVE_WIDTH 250
#define WINDOW_WIDTH (SPACE + INSTRUMENT_WIDTH + SPACE + CHANNEL_COMBO_WIDTH + SPACE)
#define WINDOW_HEIGHT (SPACE * 7 + BETWEEN_ROW_SPACE * 5 + LABEL_HEIGHT * 5 + DATA_ROW_HEIGHT * 5 + BUTTON_HEIGHT)

void open_connection_editor(Patch *p, Connection *c, KmTableListBox *connections_table)
{
  bool is_new = c == nullptr;
  if (is_new) {
    Editor e;
    c = e.create_connection(nullptr, nullptr);
  }

  DialogWindow::LaunchOptions opts;
  opts.dialogTitle = "Connection";
  opts.dialogBackgroundColour =
    LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
  opts.resizable = false;
  auto cdc = new ConnectionDialogComponent(p, c, is_new, connections_table);
  opts.content.setOwned(cdc);
  auto dialog_win = opts.launchAsync();
  if (dialog_win != nullptr)
    dialog_win->centreWithSize(cdc->width(), cdc->height());
}

ConnectionDialogComponent::ConnectionDialogComponent(
  Patch *p, Connection *c, bool is_new, KmTableListBox *connections_table)
  : _patch(p), _conn(c), _is_new(is_new), _connections_table(connections_table)
{
  init_input();
  init_output();
  init_prog();
  init_zone();
  init_xpose();
  init_velocity_curve();

  // TODO message filter, CC maps

  _ok.onClick = [this] { ok(); };
  _cancel.onClick = [this] { cancel(); };
  _apply.onClick = [this] { apply(); };

  addAndMakeVisible(_ok);
  addAndMakeVisible(_cancel);
  addAndMakeVisible(_apply);

  setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

int ConnectionDialogComponent::width() {
  return WINDOW_WIDTH;
}

int ConnectionDialogComponent::height() {
  return WINDOW_HEIGHT;
}

void ConnectionDialogComponent::resized() {
  using Track = Grid::TrackInfo;
  using Fr = Grid::Fr;
  using Px = Grid::Px;

  auto area = getLocalBounds();
  area.reduce(SPACE, SPACE);

  layout_instrument(area, _input_inst_label, _input_instrument, _input_chan_label, _input_chan);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_instrument(area, _output_inst_label, _output_instrument, _output_chan_label, _output_chan);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_program(area);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_zone(area);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_xpose_and_velocity_curve(area);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_buttons(area);
}

void ConnectionDialogComponent::layout_instrument(
  Rectangle<int> &area, Label &inst_label, ComboBox &inst, Label &chan_label, ComboBox &chan
) {
  auto row_area = area.removeFromTop(LABEL_HEIGHT);
  inst_label.setBounds(row_area.removeFromLeft(INSTRUMENT_WIDTH));
  row_area.removeFromLeft(SPACE);
  chan_label.setBounds(row_area);

  area.removeFromTop(SPACE);
  row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  inst.setBounds(row_area.removeFromLeft(INSTRUMENT_WIDTH));
  row_area.removeFromLeft(SPACE);
  chan.setBounds(row_area.removeFromLeft(CHANNEL_COMBO_WIDTH));
}

void ConnectionDialogComponent::layout_program(Rectangle<int> &area) {
  _prog_label.setBounds(area.removeFromTop(LABEL_HEIGHT));

  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _msb_label.setBounds(row_area.removeFromLeft(PROG_LABEL_WIDTH));
  row_area.removeFromLeft(SPACE);
  _msb.setBounds(row_area.removeFromLeft(PROG_WIDTH));

  row_area.removeFromLeft(SPACE);
  _lsb_label.setBounds(row_area.removeFromLeft(PROG_LABEL_WIDTH));
  row_area.removeFromLeft(SPACE);
  _lsb.setBounds(row_area.removeFromLeft(PROG_WIDTH));

  row_area.removeFromLeft(SPACE);
  _prog_field_label.setBounds(row_area.removeFromLeft(PROG_LABEL_WIDTH));
  row_area.removeFromLeft(SPACE);
  _prog.setBounds(row_area.removeFromLeft(PROG_WIDTH));
}

void ConnectionDialogComponent::layout_zone(Rectangle<int> &area) {
  _zone_label.setBounds(area.removeFromTop(LABEL_HEIGHT));

  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _zone_low.setBounds(row_area.removeFromLeft(ZONE_WIDTH));
  row_area.removeFromLeft(SPACE);
  _zone_between.setBounds(row_area.removeFromLeft(ZONE_BETWEEN_WIDTH));
  row_area.removeFromLeft(SPACE);
  _zone_high.setBounds(row_area.removeFromLeft(ZONE_WIDTH));
}

void ConnectionDialogComponent::layout_xpose_and_velocity_curve(Rectangle<int> &area) {
  auto row_area = area.removeFromTop(LABEL_HEIGHT);
  _xpose_label.setBounds(row_area.removeFromLeft(XPOSE_COLUMN_WIDTH));
  row_area.removeFromLeft(SPACE);
  _vc_label.setBounds(row_area.removeFromLeft(VCURVE_WIDTH));

  area.removeFromTop(SPACE);
  row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _xpose.setBounds(row_area.removeFromLeft(XPOSE_FIELD_WIDTH));
  row_area.removeFromLeft((XPOSE_COLUMN_WIDTH - XPOSE_FIELD_WIDTH) + SPACE);
  _vc.setBounds(row_area.removeFromLeft(VCURVE_WIDTH));
}

void ConnectionDialogComponent::layout_buttons(Rectangle<int> &area) {
  auto row_area = area.removeFromTop(BUTTON_HEIGHT);
  _ok.setBounds(row_area.removeFromRight(BUTTON_WIDTH));
  row_area.removeFromRight(SPACE);
  _apply.setBounds(row_area.removeFromRight(BUTTON_WIDTH));
  row_area.removeFromRight(SPACE);
  _cancel.setBounds(row_area.removeFromRight(BUTTON_WIDTH));
}

void ConnectionDialogComponent::init_input() {
  _input_instrument.addItem("Select input instrument", UNSELECTED);
  int i = 0;
  for (auto inp : KeyMaster_instance()->device_manager().inputs()) {
    _input_instrument.addItem(inp->info.name, i+1);
    if (inp == _conn->input())
      _input_instrument.setSelectedId(i);
    ++i;
  }
  if (_input_instrument.getSelectedId() == 0)
    _input_instrument.setSelectedId(1);

  _input_chan.addItem("All Channels", CONNECTION_ALL_CHANNELS);
  for (i = 1; i <= MIDI_CHANNELS; ++i)
    _input_chan.addItem(String(i), i);
  if (_conn->input_chan() == CONNECTION_ALL_CHANNELS)
    _input_chan.setSelectedId(CONNECTION_ALL_CHANNELS);
  else
    _input_chan.setSelectedId(_conn->input_chan() + 1);

  addAndMakeVisible(_input_inst_label);
  addAndMakeVisible(_input_instrument);
  addAndMakeVisible(_input_chan_label);
  addAndMakeVisible(_input_chan);
}

void ConnectionDialogComponent::init_output() {
  _output_instrument.addItem("Select output instrument", UNSELECTED);
  int i = 0;
  for (auto outp : KeyMaster_instance()->device_manager().outputs()) {
    _output_instrument.addItem(outp->info.name, i+1);
    if (outp == _conn->output())
      _output_instrument.setSelectedId(i);
    ++i;
  }
  if (_output_instrument.getSelectedId() == 0)
    _output_instrument.setSelectedId(1);

  _output_chan.addItem("Input Channel", CONNECTION_ALL_CHANNELS);
  for (i = 1; i <= MIDI_CHANNELS; ++i)
    _output_chan.addItem(String(i), i);
  if (_conn->output_chan() == CONNECTION_ALL_CHANNELS)
    _output_chan.setSelectedId(CONNECTION_ALL_CHANNELS);
  else
    _output_chan.setSelectedId(_conn->output_chan() + 1);

  addAndMakeVisible(_output_inst_label);
  addAndMakeVisible(_output_instrument);
  addAndMakeVisible(_output_chan_label);
  addAndMakeVisible(_output_chan);;
}

void ConnectionDialogComponent::init_prog() {
  init_text_editor(_msb, _conn->program_bank_msb() == UNDEFINED ? "" : String(_conn->program_bank_msb()));
  init_text_editor(_lsb, _conn->program_bank_lsb() == UNDEFINED ? "" : String(_conn->program_bank_lsb()));
  init_text_editor(_prog, _conn->program_prog() == UNDEFINED ? "" : String(_conn->program_prog()));

  addAndMakeVisible(_prog_label);
  addAndMakeVisible(_msb_label);
  addAndMakeVisible(_lsb_label);
  addAndMakeVisible(_prog_field_label);
}

void ConnectionDialogComponent::init_text_editor(TextEditor &te, String initial_contents) {
  te.setSelectAllWhenFocused(true);
  te.setEscapeAndReturnKeysConsumed(false);
  te.setColour(TextEditor::outlineColourId, findColour(ComboBox::outlineColourId));
  // te.setFont(getLookAndFeel().getAlertWindowMessageFont());
  te.setText(initial_contents);
  te.setCaretPosition(initial_contents.length());
  addAndMakeVisible(te);
}

void ConnectionDialogComponent::init_zone() {
  init_text_editor(_zone_low, MidiMessage::getMidiNoteName(_conn->zone_low(), true, true, 4));
  init_text_editor(_zone_high, MidiMessage::getMidiNoteName(_conn->zone_high(), true, true, 4));

  addAndMakeVisible(_zone_label);
  addAndMakeVisible(_zone_between);
}

void ConnectionDialogComponent::init_xpose() {
  init_text_editor(_xpose, String(_conn->xpose()));

  addAndMakeVisible(_xpose_label);
}

void ConnectionDialogComponent::init_velocity_curve() {
  _vc.addItem("None (Linear)", UNSELECTED);
  _vc.setSelectedId(UNSELECTED);
  int i = 0;
  for (auto *curve : KeyMaster_instance()->curves()) {
    _vc.addItem(curve->name(), ++i);
    if (curve == _conn->velocity_curve())
      _vc.setSelectedId(i);
  }

  addAndMakeVisible(_vc_label);
  addAndMakeVisible(_vc);
}

void ConnectionDialogComponent::ok() {
  if (apply())
    static_cast<DialogWindow*>(getParentComponent())->closeButtonPressed();
}

void ConnectionDialogComponent::cancel() {
  if (_is_new)
    delete _conn;
  static_cast<DialogWindow*>(getParentComponent())->closeButtonPressed();
}

// Displays an alert and returns false if apply fails.
bool ConnectionDialogComponent::apply() {
  Array<String> error_msgs;

  DeviceManager &dm = KeyMaster_instance()->device_manager();
  int id = _input_instrument.getSelectedId();
  Input::Ptr input = id == UNSELECTED ? nullptr : dm.inputs()[id-1];

  int input_chan = _input_chan.getSelectedId();
  if (input_chan != CONNECTION_ALL_CHANNELS)
    --input_chan;

  id = _output_instrument.getSelectedId();
  Output::Ptr output = id == UNSELECTED ? nullptr : dm.outputs()[id-1];

  int output_chan = _output_chan.getSelectedId();
  if (output_chan != CONNECTION_ALL_CHANNELS)
    --output_chan;

  auto text = _msb.getText();
  int msb = text.isEmpty() ? UNDEFINED : text.getIntValue();
  if (msb != UNDEFINED && (msb < 0 || msb > 127))
    error_msgs.add("Bank MSB value must be 0-127");

  text = _lsb.getText();
  int lsb = text.isEmpty() ? UNDEFINED : text.getIntValue();
  if (lsb != UNDEFINED && (lsb < 0 || lsb > 127))
    error_msgs.add("Bank LSB value must be 0-127");

  text = _prog.getText();
  int prog = text.isEmpty() ? UNDEFINED : text.getIntValue();
  if (prog != UNDEFINED && (prog < 0 || prog > 127))
    error_msgs.add("Program value must be 0-127");

  int zone_low = note_name_to_num(_zone_low.getText());
  if (zone_low < 0 || zone_low > 127)
    error_msgs.add("Zone low must be 0 (C-1) - 127 (G8)");

  int zone_high = note_name_to_num(_zone_high.getText());
  if (zone_high < 0 || zone_high > 127)
    error_msgs.add("Zone high must be 0 (C0) - 127 (G9)");

  int xpose = _xpose.getText().getIntValue();

  id = _vc.getSelectedId();
  Curve *curve = nullptr;
  if (id != UNSELECTED)
    curve = KeyMaster_instance()->curves()[id-1];

  // TODO message filter, CC maps

  if (!error_msgs.isEmpty()) {
    String message = "The following errors prevent the connection from being saved:\n";
    for (auto err : error_msgs) {
      message << "\n- ";
      message << err;
    }
    AlertWindow::showMessageBoxAsync(MessageBoxIconType::WarningIcon,
                                     "Connection Edit Errors", message,
                                     "OK", this);
    return false;
  }

  _conn->begin_changes();
  _conn->set_input(input);
  _conn->set_input_chan(input_chan);
  _conn->set_output(output);
  _conn->set_output_chan(output_chan);
  _conn->set_program_bank_msb(msb);
  _conn->set_program_bank_lsb(lsb);
  _conn->set_program_prog(prog);
  _conn->set_zone_low(zone_low);
  _conn->set_zone_high(zone_high);
  _conn->set_xpose(xpose);
  _conn->set_velocity_curve(curve);
  _conn->end_changes();

  if (_is_new) {
    _patch->add_connection(_conn);
    _is_new = false;
  }

  // FIXME not updating if not just created; conn not sending changed to KM instance?
  _connections_table->updateContent();
  _connections_table->repaint();
  return true;
}
