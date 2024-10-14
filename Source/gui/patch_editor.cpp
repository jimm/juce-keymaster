#include "patch_editor.h"
#include "../km/consts.h"
#include "../km/keymaster.h"
#include "../km/patch.h"
#include "../km/editor.h"

#define NAME_WIDTH 300
#define MESSAGE_MENU_WIDTH 200

#define CONTENT_WIDTH (NAME_WIDTH)
#define CONTENT_HEIGHT (SPACE * 3 + LABEL_HEIGHT * 3 + DATA_ROW_HEIGHT * 3 + BETWEEN_ROW_SPACE * 2)

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
  : KmEditor(is_new), _patch(p)
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

void PatchEditor::init() {
  _name.setText(_patch->name());
  addAndMakeVisible(_name_label);
  addAndMakeVisible(_name);

  init_message(_patch->start_message(), _start_label, _start);
  init_message(_patch->stop_message(), _stop_label, _stop);

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

void PatchEditor::cancel_cleanup() {
  delete _patch;
}

bool PatchEditor::apply() {
  _patch->set_name(_name.getText());

  Array<MessageBlock *>messages = KeyMaster_instance()->messages();
  int sel_id = _start.getSelectedId();
  _patch->set_start_message((sel_id == UNSELECTED) ? nullptr : messages[sel_id - 1]);
  sel_id = _stop.getSelectedId();
  _patch->set_stop_message((sel_id == UNSELECTED) ? nullptr : messages[sel_id - 1]);

  if (_is_new) {
    Editor e;
    e.add_patch(_patch);
    _is_new = false;
  }

  sendActionMessage("update:list-box");
  return true;
}
