#include "set_list_editor.h"
#include "../km/consts.h"
#include "../km/keymaster.h"
#include "../km/set_list.h"
#include "../km/editor.h"

#define NAME_WIDTH 300
#define LIST_WIDTH 150
#define LIST_HEIGHT 300

#define CONTENT_WIDTH (LIST_WIDTH * 2 + SPACE)
#define CONTENT_HEIGHT (BETWEEN_ROW_SPACE * 1 + LIST_HEIGHT)

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
  layout_lists(area);
}

void SetListEditor::layout_lists(Rectangle<int> &area) {
  area.removeFromTop(BETWEEN_ROW_SPACE);
  // TODO label
  _all_songs.setBounds(area.removeFromLeft(LIST_WIDTH));

  area.removeFromLeft(SPACE);
  // TODO set list
  // _set_list_list.setBounds(area);

  KmEditor::layout(area);
}

void SetListEditor::layout_name(Rectangle<int> &area) {
  _name_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _name.setBounds(row_area.removeFromLeft(NAME_WIDTH));
}

void SetListEditor::init() {
  _name.setText(_set_list->name());

  _all_songs.setModel(&_all_songs_model);
  _all_songs.setMultipleSelectionEnabled(true);

  addAndMakeVisible(_name_label);
  addAndMakeVisible(_name);
  // TODO addAndMakeVisible(_all_songs_label);
  addAndMakeVisible(_all_songs);
  // TODO addAndMakeVisible(_set_list_list);

  KmEditor::init();
}

void SetListEditor::cancel_cleanup() {
  delete _set_list;
}

bool SetListEditor::apply() {
  _set_list->set_name(_name.getText());

  if (_is_new) {
    Editor e;
    e.add_set_list(_set_list);
    _is_new = false;
  }

  sendActionMessage("update:all");
  return true;
}
