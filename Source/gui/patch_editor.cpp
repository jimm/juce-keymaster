#include "patch_editor.h"
#include "../km/consts.h"
#include "../km/keymaster.h"
#include "../km/patch.h"
#include "../km/editor.h"

#define NAME_WIDTH 300
#define IPC_TABLE_WIDTH 460
#define IPC_TABLE_HEIGHT 120
#define IPC_BUTTON_WIDTH 40
#define IPC_BUTTON_HEIGHT 25
#define MESSAGE_MENU_WIDTH 200

#define CONTENT_WIDTH IPC_TABLE_WIDTH
#define CONTENT_HEIGHT (SPACE * 5 + LABEL_HEIGHT * 4 + DATA_ROW_HEIGHT * 3 + \
  BETWEEN_ROW_SPACE * 3 + IPC_TABLE_HEIGHT + IPC_BUTTON_HEIGHT)

// ================================================================
// IpcTableModel
// ================================================================

int IpcTableModel::getNumRows() {
  return _ipcs.size();
}

void IpcTableModel::paintRowBackground(
  Graphics &g, int /*row*/, int w, int h, bool sel)
{
  if (sel)
    g.fillAll(LookAndFeel::getDefaultLookAndFeel()
                .findColour(TextEditor::highlightColourId));
}

Component* IpcTableModel::refreshComponentForCell(
  int row, int col, bool /*sel*/, Component *existing)
{
  delete existing;
  if (row >= _ipcs.size())
    return nullptr;

  auto &ipc = _ipcs.getReference(row);

  switch (col) {
  case 1: {  // Output
    auto cb = new ComboBox();
    int sel_id = 0;
    int i = 1;
    for (auto out : KeyMaster_instance()->device_manager().outputs()) {
      cb->addItem(out->name(), i);
      if (out == ipc.output)
        sel_id = i;
      ++i;
    }
    if (sel_id == 0 && cb->getNumItems() > 0) {
      sel_id = 1;
      ipc.output = KeyMaster_instance()->device_manager().outputs()[0];
    }
    cb->setSelectedId(sel_id, dontSendNotification);
    cb->onChange = [this, row, cb] {
      if (row >= _ipcs.size()) return;
      int id = cb->getSelectedId();
      if (id > 0)
        _ipcs.getReference(row).output =
          KeyMaster_instance()->device_manager().outputs()[id - 1];
    };
    return cb;
  }
  case 2: {  // Channel
    auto cb = new ComboBox();
    for (int ch = 1; ch <= MIDI_CHANNELS; ++ch)
      cb->addItem(String(ch), ch);
    cb->setSelectedId(ipc.channel > 0 ? ipc.channel : 1, dontSendNotification);
    cb->onChange = [this, row, cb] {
      if (row >= _ipcs.size()) return;
      _ipcs.getReference(row).channel = cb->getSelectedId();
    };
    return cb;
  }
  case 3: {  // Bank MSB
    auto te = new TextEditor();
    te->setText(ipc.bank_msb == UNDEFINED ? "" : String(ipc.bank_msb),
                dontSendNotification);
    te->onTextChange = [this, row, te] {
      if (row >= _ipcs.size()) return;
      auto text = te->getText();
      _ipcs.getReference(row).bank_msb =
        text.isEmpty() ? UNDEFINED : text.getIntValue();
    };
    return te;
  }
  case 4: {  // Bank LSB
    auto te = new TextEditor();
    te->setText(ipc.bank_lsb == UNDEFINED ? "" : String(ipc.bank_lsb),
                dontSendNotification);
    te->onTextChange = [this, row, te] {
      if (row >= _ipcs.size()) return;
      auto text = te->getText();
      _ipcs.getReference(row).bank_lsb =
        text.isEmpty() ? UNDEFINED : text.getIntValue();
    };
    return te;
  }
  case 5: {  // Prog
    auto te = new TextEditor();
    te->setText(ipc.prog == UNDEFINED ? "" : String(ipc.prog),
                dontSendNotification);
    te->onTextChange = [this, row, te] {
      if (row >= _ipcs.size()) return;
      auto text = te->getText();
      _ipcs.getReference(row).prog =
        text.isEmpty() ? UNDEFINED : text.getIntValue();
    };
    return te;
  }
  default:
    return nullptr;
  }
}

// ================================================================
// PatchEditor
// ================================================================

PatchEditor * open_patch_editor(Patch *p)
{
  bool is_new = p == nullptr;
  if (is_new) {
    Editor e;
    p = e.create_patch();
  }

  DialogWindow::LaunchOptions opts;
  opts.dialogTitle = "Patch";
  opts.dialogBackgroundColour =
    LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
  opts.resizable = false;
  auto cdc = new PatchEditor(p, is_new);
  opts.content.setOwned(cdc);
  auto dialog_win = opts.launchAsync();
  if (dialog_win != nullptr)
    dialog_win->centreWithSize(cdc->width(), cdc->height());
  return cdc;
}

PatchEditor::PatchEditor(Patch *p, bool is_new)
  : KmEditor(is_new), _patch(p), _ipc_model(_edit_ipcs)
{
  init();
}

int PatchEditor::width() {
  return KmEditor::width() + CONTENT_WIDTH;
}

int PatchEditor::height() {
  return KmEditor::height() + CONTENT_HEIGHT;
}

void PatchEditor::layout(Rectangle<int> &area) {
  layout_name(area);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_message(area, _start_label, _start);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_message(area, _stop_label, _stop);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_ipc_table(area);

  KmEditor::layout(area);
}

void PatchEditor::layout_name(Rectangle<int> &area) {
  _name_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _name.setBounds(row_area.removeFromLeft(NAME_WIDTH));
}

void PatchEditor::layout_message(Rectangle<int> &area, Label &label, ComboBox &combo_box) {
  label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  combo_box.setBounds(row_area.removeFromLeft(MESSAGE_MENU_WIDTH));
}

void PatchEditor::layout_ipc_table(Rectangle<int> &area) {
  _ipc_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  _ipc_table.setBounds(area.removeFromTop(IPC_TABLE_HEIGHT));
  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(IPC_BUTTON_HEIGHT);
  _add_ipc.setBounds(row_area.removeFromLeft(IPC_BUTTON_WIDTH));
  row_area.removeFromLeft(SPACE);
  _del_ipc.setBounds(row_area.removeFromLeft(IPC_BUTTON_WIDTH));
}

void PatchEditor::init() {
  _edit_ipcs = _patch->instrument_program_changes();

  _name.setText(_patch->name());
  addAndMakeVisible(_name_label);
  addAndMakeVisible(_name);

  init_message(_patch->start_message(), _start_label, _start);
  init_message(_patch->stop_message(), _stop_label, _stop);
  init_ipc_table();

  KmEditor::init();
}

void PatchEditor::init_message(MessageBlock *mb, Label &label, ComboBox &combo_box) {
  combo_box.addItem("(None)", UNSELECTED);
  int i = 1;
  for (auto msg : KeyMaster_instance()->messages()) {
    combo_box.addItem(msg->name(), i);
    if (msg == mb)
      combo_box.setSelectedId(i);
    ++i;
  }
  addAndMakeVisible(label);
  addAndMakeVisible(combo_box);
}

void PatchEditor::init_ipc_table() {
  auto &header = _ipc_table.getHeader();
  header.addColumn("Output", 1, 190, 100, -1,
                   TableHeaderComponent::visible | TableHeaderComponent::resizable);
  header.addColumn("Chan",     2,  55, 40, -1,
                   TableHeaderComponent::visible | TableHeaderComponent::resizable);
  header.addColumn("Bank MSB", 3,  65, 40, -1,
                   TableHeaderComponent::visible | TableHeaderComponent::resizable);
  header.addColumn("Bank LSB", 4,  65, 40, -1,
                   TableHeaderComponent::visible | TableHeaderComponent::resizable);
  header.addColumn("Prog",     5,  65, 40, -1,
                   TableHeaderComponent::visible | TableHeaderComponent::resizable);

  _ipc_table.setModel(&_ipc_model);
  _ipc_table.setColour(ListBox::outlineColourId, Colours::grey);
  _ipc_table.setOutlineThickness(1);

  _add_ipc.onClick = [this] { add_ipc(); };
  _del_ipc.onClick = [this] { del_ipc(); };

  addAndMakeVisible(_ipc_label);
  addAndMakeVisible(_ipc_table);
  addAndMakeVisible(_add_ipc);
  addAndMakeVisible(_del_ipc);
}

void PatchEditor::cancel_cleanup() {
  delete _patch;
}

void PatchEditor::add_ipc() {
  InstrumentProgramChange ipc;
  auto &outputs = KeyMaster_instance()->device_manager().outputs();
  if (!outputs.isEmpty())
    ipc.output = outputs[0];
  _edit_ipcs.add(ipc);
  _ipc_table.updateContent();
  _ipc_table.repaint();
}

void PatchEditor::del_ipc() {
  int row = _ipc_table.getSelectedRow();
  if (row >= 0 && row < _edit_ipcs.size()) {
    _edit_ipcs.remove(row);
    _ipc_table.updateContent();
    _ipc_table.repaint();
  }
}

bool PatchEditor::apply() {
  _patch->set_name(_name.getText());

  Array<MessageBlock *> messages = KeyMaster_instance()->messages();
  int sel_id = _start.getSelectedId();
  _patch->set_start_message((sel_id == UNSELECTED) ? nullptr : messages[sel_id - 1]);
  sel_id = _stop.getSelectedId();
  _patch->set_stop_message((sel_id == UNSELECTED) ? nullptr : messages[sel_id - 1]);

  StringArray duplicates;
  for (int i = 0; i < _edit_ipcs.size() - 1; ++i) {
    for (int j = i + 1; j < _edit_ipcs.size(); ++j) {
      if (_edit_ipcs[i].output != nullptr && _edit_ipcs[j].output != nullptr
              && _edit_ipcs[i].output->identifier() == _edit_ipcs[j].output->identifier()
              && _edit_ipcs[i].channel == _edit_ipcs[j].channel) {
        String desc = _edit_ipcs[i].output->name() + " ch " + String(_edit_ipcs[i].channel);
        if (!duplicates.contains(desc))
          duplicates.add(desc);
      }
    }
  }
  if (!duplicates.isEmpty()) {
    AlertWindow::showMessageBoxAsync(
      MessageBoxIconType::WarningIcon, "Duplicate Program Changes",
      "Warning: duplicate program change entries for: " + duplicates.joinIntoString(", "));
  }

  _patch->set_instrument_program_changes(_edit_ipcs);

  if (_is_new) {
    Editor e;
    e.add_patch(_patch);
    _is_new = false;
  }

  sendActionMessage("update:list-box");
  return true;
}
