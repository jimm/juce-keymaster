#include "set_list_editor.h"
#include "../km/consts.h"
#include "../km/keymaster.h"
#include "../km/set_list.h"
#include "../km/editor.h"

#define NAME_WIDTH 300
#define LIST_WIDTH 150
#define LIST_HEIGHT 300

#define CONTENT_WIDTH (LIST_WIDTH * 2 + SPACE)
#define CONTENT_HEIGHT (BETWEEN_ROW_SPACE * 1 + SPACE * 2 + DATA_ROW_HEIGHT + LABEL_HEIGHT * 2 + LIST_HEIGHT)

SetListEditor * open_set_list_editor(SetList *sl)
{
  bool is_new = sl == nullptr;
  if (is_new) {
    Editor e;
    sl = e.create_set_list();
  }

  DialogWindow::LaunchOptions opts;
  opts.dialogTitle = "Set List";
  opts.dialogBackgroundColour =
    LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
  opts.resizable = false;
  auto cdc = new SetListEditor(sl, is_new);
  opts.content.setOwned(cdc);
  auto dialog_win = opts.launchAsync();
  if (dialog_win != nullptr)
    dialog_win->centreWithSize(cdc->width(), cdc->height());
  return cdc;
}

SetListEditor::SetListEditor(SetList *sl, bool is_new)
  : KmEditor(is_new), _set_list(sl)
{
  init();
}

int SetListEditor::width() {
  return KmEditor::width() + CONTENT_WIDTH;
}

int SetListEditor::height() {
  return KmEditor::height() + CONTENT_HEIGHT;
}

void SetListEditor::layout(Rectangle<int> &area) {
  layout_name(area);
  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_lists(area);
  KmEditor::layout(area);
}

void SetListEditor::layout_name(Rectangle<int> &area) {
  _name_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _name.setBounds(row_area.removeFromLeft(NAME_WIDTH));
}

void SetListEditor::layout_lists(Rectangle<int> &area) {
  auto row_area = area.removeFromTop(LABEL_HEIGHT);
  _all_songs_label.setBounds(row_area.removeFromLeft(LIST_WIDTH));
  row_area.removeFromLeft(SPACE);
  _set_list_label.setBounds(row_area);

  area.removeFromTop(SPACE);
  row_area = area.removeFromTop(LIST_HEIGHT);
  _all_songs_box.setBounds(row_area.removeFromLeft(LIST_WIDTH));
  row_area.removeFromLeft(SPACE);
  _set_list_box.setBounds(row_area);
}

void SetListEditor::init() {
  _name.setText(_set_list->name());

  _all_songs_box.setModel(&_all_songs_model);
  _all_songs_box.setMultipleSelectionEnabled(true);

  _set_list_box.setModel(&_set_list_model);
  _set_list_box.setMultipleSelectionEnabled(false);

  addAndMakeVisible(_name_label);
  addAndMakeVisible(_name);
  addAndMakeVisible(_all_songs_label);
  addAndMakeVisible(_all_songs_box);
  addAndMakeVisible(_set_list_label);
  addAndMakeVisible(_set_list_box);

  KmEditor::init();
}

void SetListEditor::cancel_cleanup() {
  delete _set_list;
}

bool SetListEditor::apply() {
  _set_list->set_name(_name.getText());

  auto model = static_cast<SetListModel *>(_set_list_box.getListBoxModel());
  _set_list->set_songs(model->set_list()->songs());

  if (_is_new) {
    Editor e;
    e.add_set_list(_set_list);
    _is_new = false;
  }

  sendActionMessage("update:all");
  return true;
}
