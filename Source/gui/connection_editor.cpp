#include "connection_editor.h"
#include "cc_map_editor.h"
#include "../km/consts.h"
#include "../km/keymaster.h"
#include "../km/patch.h"
#include "../km/connection.h"
#include "../km/editor.h"
#include "../km/formatter.h"

#define INSTRUMENT_WIDTH 250
#define CHANNEL_COMBO_WIDTH 125
#define PROG_LABEL_WIDTH 35
#define PROG_WIDTH 40
#define ZONE_WIDTH 40
#define ZONE_BETWEEN_WIDTH 16
#define XPOSE_COLUMN_WIDTH 100
#define XPOSE_FIELD_WIDTH 40
#define VCURVE_WIDTH 250

#define MESSAGE_FILTER_CHECKBOX_WIDTH 150
#define MESSAGE_FILTER_CHECKBOX_HEIGHT 18

#define CC_MAPS_TABLE_HEIGHT 150
#define CC_MAPS_TABLE_WIDTH 300
#define CC_MAPS_BUTTON_HEIGHT 25
#define CC_MAPS_BUTTON_WIDTH 40

#define CONTENT_WIDTH (CC_MAPS_TABLE_WIDTH * 2)
#define CONTENT_HEIGHT (SPACE * 8 + BETWEEN_ROW_SPACE * 7 + LABEL_HEIGHT * 7 + DATA_ROW_HEIGHT * 5 + MESSAGE_FILTER_CHECKBOX_HEIGHT * 7 + CC_MAPS_TABLE_HEIGHT + CC_MAPS_BUTTON_HEIGHT)

ConnectionEditor * open_connection_editor(Connection *c)
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
  auto cdc = new ConnectionEditor(c, is_new);
  opts.content.setOwned(cdc);
  auto dialog_win = opts.launchAsync();
  if (dialog_win != nullptr)
    dialog_win->centreWithSize(cdc->width(), cdc->height());
  return cdc;
}

ConnectionEditor::ConnectionEditor(
  Connection *c, bool is_new)
  : KmEditor(is_new), _conn(c)
{
  init_input();
  init_output();
  init_prog();
  init_zone();
  init_xpose();
  init_message_filters();
  init_velocity_curve();
  init_cc_maps();
  update_enabled_states();

  _add_cc_map.onClick = [this] { add_cc_map(); };
  _del_cc_map.onClick = [this] { del_cc_map(); };

  KmEditor::init();
}

int ConnectionEditor::width() {
  return KmEditor::width() + CONTENT_WIDTH;
}

int ConnectionEditor::height() {
  return KmEditor::height() + CONTENT_HEIGHT;
}

void ConnectionEditor::layout(Rectangle<int> &area) {
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
  layout_message_filters(area);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_cc_maps(area);

  KmEditor::layout(area);
}

void ConnectionEditor::layout_instrument(
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

void ConnectionEditor::layout_program(Rectangle<int> &area) {
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

void ConnectionEditor::layout_zone(Rectangle<int> &area) {
  _zone_label.setBounds(area.removeFromTop(LABEL_HEIGHT));

  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _zone_low.setBounds(row_area.removeFromLeft(ZONE_WIDTH));
  row_area.removeFromLeft(SPACE);
  _zone_between.setBounds(row_area.removeFromLeft(ZONE_BETWEEN_WIDTH));
  row_area.removeFromLeft(SPACE);
  _zone_high.setBounds(row_area.removeFromLeft(ZONE_WIDTH));
}

void ConnectionEditor::layout_xpose_and_velocity_curve(Rectangle<int> &area) {
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

void ConnectionEditor::layout_toggle_row(
  Rectangle<int> &area, ToggleButton &left, ToggleButton &right)
{
  auto row_area = area.removeFromTop(MESSAGE_FILTER_CHECKBOX_HEIGHT);
  left.setBounds(row_area.removeFromLeft(MESSAGE_FILTER_CHECKBOX_WIDTH));
  row_area.removeFromLeft(SPACE);
  right.setBounds(row_area.removeFromLeft(MESSAGE_FILTER_CHECKBOX_WIDTH));
}

void ConnectionEditor::layout_message_filters(Rectangle<int> &area) {
  _filters_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);

  layout_toggle_row(area, _filt_note, _filt_sptr);
  layout_toggle_row(area, _filt_poly_press, _filt_ssel);
  layout_toggle_row(area, _filt_chan_press, _filt_tune);
  layout_toggle_row(area, _filt_prog, _filt_sysex);
  layout_toggle_row(area, _filt_bend, _filt_clock);
  layout_toggle_row(area, _filt_ctrl, _filt_start);
  auto row_area = area.removeFromTop(MESSAGE_FILTER_CHECKBOX_HEIGHT);
  row_area.removeFromLeft(MESSAGE_FILTER_CHECKBOX_WIDTH + SPACE);
  _filt_reset.setBounds(row_area.removeFromLeft(MESSAGE_FILTER_CHECKBOX_WIDTH));
}

void ConnectionEditor::layout_cc_maps(Rectangle<int> &area) {
  _cc_maps_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  _cc_maps_list_box.setBounds(area.removeFromTop(CC_MAPS_TABLE_HEIGHT));
  area.removeFromTop(SPACE);

  auto row_area = area.removeFromTop(BUTTON_HEIGHT);
  _add_cc_map.setBounds(row_area.removeFromLeft(CC_MAPS_BUTTON_WIDTH));
  row_area.removeFromLeft(SPACE);
  _del_cc_map.setBounds(row_area.removeFromLeft(CC_MAPS_BUTTON_WIDTH));
}

void ConnectionEditor::init_input() {
  _input_instrument.addItem("Select input instrument", UNSELECTED);
  int i = 0;
  for (auto inp : KeyMaster_instance()->device_manager().inputs()) {
    _input_instrument.addItem(inp->info.name, i+1);
    if (inp == _conn->input())
      _input_instrument.setSelectedId(i);
    ++i;
  }
  if (_input_instrument.getSelectedId() == 0)
    _input_instrument.setSelectedId(UNSELECTED);

  _input_chan.addItem("All Channels", CONNECTION_ALL_CHANNELS);
  for (i = 1; i <= MIDI_CHANNELS; ++i)
    _input_chan.addItem(String(i), i);
  if (_conn->input_chan() == CONNECTION_ALL_CHANNELS)
    _input_chan.setSelectedId(CONNECTION_ALL_CHANNELS);
  else
    _input_chan.setSelectedId(_conn->input_chan() + 1);

  _input_chan.addActionListener(this);

  addAndMakeVisible(_input_inst_label);
  addAndMakeVisible(_input_instrument);
  addAndMakeVisible(_input_chan_label);
  addAndMakeVisible(_input_chan);
}

void ConnectionEditor::init_output() {
  _output_instrument.addItem("Select output instrument", UNSELECTED);
  int i = 0;
  for (auto outp : KeyMaster_instance()->device_manager().outputs()) {
    _output_instrument.addItem(outp->info.name, i+1);
    if (outp == _conn->output())
      _output_instrument.setSelectedId(i);
    ++i;
  }
  if (_output_instrument.getSelectedId() == 0)
    _output_instrument.setSelectedId(UNSELECTED);

  _output_chan.addItem("Input Channel", CONNECTION_ALL_CHANNELS);
  for (i = 1; i <= MIDI_CHANNELS; ++i)
    _output_chan.addItem(String(i), i);
  if (_conn->output_chan() == CONNECTION_ALL_CHANNELS)
    _output_chan.setSelectedId(CONNECTION_ALL_CHANNELS);
  else
    _output_chan.setSelectedId(_conn->output_chan() + 1);

  _output_chan.addActionListener(this);

  addAndMakeVisible(_output_inst_label);
  addAndMakeVisible(_output_instrument);
  addAndMakeVisible(_output_chan_label);
  addAndMakeVisible(_output_chan);;
}

void ConnectionEditor::init_prog() {
  init_text_editor(_msb, _conn->program_bank_msb() == UNDEFINED ? "" : String(_conn->program_bank_msb()));
  init_text_editor(_lsb, _conn->program_bank_lsb() == UNDEFINED ? "" : String(_conn->program_bank_lsb()));
  init_text_editor(_prog, _conn->program_prog() == UNDEFINED ? "" : String(_conn->program_prog()));

  addAndMakeVisible(_prog_label);
  addAndMakeVisible(_msb_label);
  addAndMakeVisible(_lsb_label);
  addAndMakeVisible(_prog_field_label);
}

void ConnectionEditor::init_text_editor(TextEditor &te, String initial_contents) {
  te.setSelectAllWhenFocused(true);
  te.setEscapeAndReturnKeysConsumed(false);
  te.setColour(TextEditor::outlineColourId, findColour(ComboBox::outlineColourId));
  te.setText(initial_contents);
  te.setCaretPosition(initial_contents.length());
  addAndMakeVisible(te);
}

void ConnectionEditor::init_zone() {
  init_text_editor(_zone_low, MidiMessage::getMidiNoteName(_conn->zone_low(), true, true, 4));
  init_text_editor(_zone_high, MidiMessage::getMidiNoteName(_conn->zone_high(), true, true, 4));

  addAndMakeVisible(_zone_label);
  addAndMakeVisible(_zone_between);
}

void ConnectionEditor::init_xpose() {
  init_text_editor(_xpose, String(_conn->xpose()));

  addAndMakeVisible(_xpose_label);
}

void ConnectionEditor::init_message_filters() {
  auto &mf = _conn->message_filter();

  auto ignore = NotificationType::dontSendNotification;
  _filt_note.setToggleState(mf.note(), ignore);
  _filt_poly_press.setToggleState(mf.poly_pressure(), ignore);
  _filt_chan_press.setToggleState(mf.chan_pressure(), ignore);
  _filt_prog.setToggleState(mf.program_change(), ignore);
  _filt_bend.setToggleState(mf.pitch_bend(), ignore);
  _filt_ctrl.setToggleState(mf.controller(), ignore);
  _filt_sptr.setToggleState(mf.song_pointer(), ignore);
  _filt_ssel.setToggleState(mf.song_select(), ignore);
  _filt_tune.setToggleState(mf.tune_request(), ignore);
  _filt_sysex.setToggleState(mf.sysex(), ignore);
  _filt_clock.setToggleState(mf.clock(), ignore);
  _filt_start.setToggleState(mf.start_continue_stop(), ignore);
  _filt_reset.setToggleState(mf.system_reset(), ignore);

  addAndMakeVisible(_filters_label);
  addAndMakeVisible(_filt_note);
  addAndMakeVisible(_filt_poly_press);
  addAndMakeVisible(_filt_chan_press);
  addAndMakeVisible(_filt_prog);
  addAndMakeVisible(_filt_bend);
  addAndMakeVisible(_filt_ctrl);
  addAndMakeVisible(_filt_sptr);
  addAndMakeVisible(_filt_ssel);
  addAndMakeVisible(_filt_tune);
  addAndMakeVisible(_filt_sysex);
  addAndMakeVisible(_filt_clock);
  addAndMakeVisible(_filt_start);
  addAndMakeVisible(_filt_reset);
}

void ConnectionEditor::init_velocity_curve() {
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

void ConnectionEditor::init_cc_maps() {
  _cc_maps_model = new CcMapsTableListBoxModel();
  _cc_maps_model->set_connection(_conn);
  _cc_maps_model->make_columns(_cc_maps_list_box.getHeader());
  _cc_maps_list_box.setModel(_cc_maps_model);
  _cc_maps_list_box.setColour(ListBox::outlineColourId, Colours::grey);
  _cc_maps_list_box.setOutlineThickness(1);
   _cc_maps_model->addActionListener(&_cc_maps_list_box);

  addAndMakeVisible(_cc_maps_label);
  addAndMakeVisible(_cc_maps_list_box);
  addAndMakeVisible(_add_cc_map);
  addAndMakeVisible(_del_cc_map);
}

void ConnectionEditor::update_enabled_states() {
  bool enable =
    _input_chan.getSelectedId() != CONNECTION_ALL_CHANNELS ||
    _output_chan.getSelectedId() != CONNECTION_ALL_CHANNELS;

  _prog_label.setEnabled(enable);
  _msb_label.setEnabled(enable);
  _msb.setEnabled(enable);
  _lsb_label.setEnabled(enable);
  _lsb.setEnabled(enable);
  _prog_field_label.setEnabled(enable);
  _prog.setEnabled(enable);
}

void ConnectionEditor::cancel_cleanup() {
  delete _conn;
}

// Displays an alert and returns false if apply fails.
bool ConnectionEditor::apply() {
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
    add_error_message("Bank MSB value must be 0-127");

  text = _lsb.getText();
  int lsb = text.isEmpty() ? UNDEFINED : text.getIntValue();
  if (lsb != UNDEFINED && (lsb < 0 || lsb > 127))
    add_error_message("Bank LSB value must be 0-127");

  text = _prog.getText();
  int prog = text.isEmpty() ? UNDEFINED : text.getIntValue();
  if (prog != UNDEFINED && (prog < 0 || prog > 127))
    add_error_message("Program value must be 0-127");

  int zone_low = note_name_to_num(_zone_low.getText());
  if (zone_low < 0 || zone_low > 127)
    add_error_message("Zone low must be 0 (C-1) - 127 (G8)");

  int zone_high = note_name_to_num(_zone_high.getText());
  if (zone_high < 0 || zone_high > 127)
    add_error_message("Zone high must be 0 (C0) - 127 (G9)");

  int xpose = _xpose.getText().getIntValue();

  id = _vc.getSelectedId();
  Curve *curve = nullptr;
  if (id != UNSELECTED)
    curve = KeyMaster_instance()->curves()[id-1];

  if (has_errors()) {
    display_errors("Connection");
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

  update_conn_cc_maps();

  auto &mf = _conn->message_filter();
  mf.set_note(_filt_note.getToggleState());
  mf.set_poly_pressure(_filt_poly_press.getToggleState());
  mf.set_chan_pressure(_filt_chan_press.getToggleState());
  mf.set_program_change(_filt_prog.getToggleState());
  mf.set_pitch_bend(_filt_bend.getToggleState());
  mf.set_controller(_filt_ctrl.getToggleState());
  mf.set_song_pointer(_filt_sptr.getToggleState());
  mf.set_song_select(_filt_ssel.getToggleState());
  mf.set_tune_request(_filt_tune.getToggleState());
  mf.set_sysex(_filt_sysex.getToggleState());
  mf.set_clock(_filt_clock.getToggleState());
  mf.set_start_continue_stop(_filt_start.getToggleState());
  mf.set_system_reset(_filt_reset.getToggleState());

  _conn->end_changes();

  if (_is_new) {
    Editor e;
    e.add_connection(KeyMaster_instance()->cursor()->patch(), _conn);
    _is_new = false;
    sendActionMessage("update:all");
  }
  else
    sendActionMessage("update:table-list-box");

  return true;
}

void ConnectionEditor::add_cc_map() {
  Controller *c = new Controller(UNDEFINED, 0);
  open_cc_map_editor(_conn, c)->addActionListener(&_cc_maps_list_box);
}

void ConnectionEditor::del_cc_map() {
  auto rows = _cc_maps_list_box.getSelectedRows();
  if (rows.size() > 0) {
    Controller *c = _cc_maps_model->nth_cc_map(rows[0]);
    delete c;
    _cc_maps_list_box.updateContent();
    _cc_maps_list_box.repaint();
  }
}

void ConnectionEditor::update_conn_cc_maps() {
  int num_cc_maps = _cc_maps_model->getNumRows();
  for (int i = 0; i < num_cc_maps; ++i) {
    auto controller = _cc_maps_model->nth_cc_map(i);
    _conn->set_cc_map(controller->cc_num(), controller);
  }

  // Get rid of controllers that are in the connection but not in the new
  // model's list.
  for (int i = 0; i < 128; ++i) {
    if (_conn->cc_map(i) == nullptr)
      break;

    bool is_in_new_list = false;
    for (int j = 0; i < num_cc_maps; ++i) {
      auto controller = _cc_maps_model->nth_cc_map(j);
      if (controller->cc_num() == i) {
        is_in_new_list = true;
        break;
      }
    }

    if (!is_in_new_list) {
      // This CC number has an old controller that is not in the new list.
      // Delete it.
      _conn->set_cc_map(i, nullptr); // takes care of deleting old one
    }
  }
}

void ConnectionEditor::actionListenerCallback(const String &message) {
  if (message == "combo:changed")
    update_enabled_states();
}
