#include "find_dialog.h"
#include "../km/keymaster.h"

#define SPACE 12
#define LABEL_HEIGHT 16
#define DATA_ROW_HEIGHT 24
#define BETWEEN_ROW_SPACE 20
#define BUTTON_HEIGHT 35
#define BUTTON_WIDTH 80
#define INPUT_WIDTH 300

#define CONTENT_WIDTH INPUT_WIDTH
#define CONTENT_HEIGHT \
  (LABEL_HEIGHT + SPACE + DATA_ROW_HEIGHT + BETWEEN_ROW_SPACE + BUTTON_HEIGHT)

void open_find_dialog(FindDialog::Type type) {
  DialogWindow::LaunchOptions opts;
  opts.dialogTitle =
    (type == FindDialog::FindSong) ? "Find a Song" : "Find a Set List";
  opts.dialogBackgroundColour =
    LookAndFeel::getDefaultLookAndFeel()
      .findColour(ResizableWindow::backgroundColourId);
  opts.resizable = false;
  auto cdc = new FindDialog(type);
  opts.content.setOwned(cdc);
  auto dialog_win = opts.launchAsync();
  if (dialog_win != nullptr)
    dialog_win->centreWithSize(cdc->width(), cdc->height());
}

FindDialog::FindDialog(Type type)
  : _type(type)
{
  _regex.onReturnKey = [this] { ok(); };

  addAndMakeVisible(_regex_label);
  addAndMakeVisible(_regex);

  _ok.onClick = [this] { ok(); };
  _cancel.onClick = [this] { cancel(); };

  addAndMakeVisible(_ok);
  addAndMakeVisible(_cancel);

  setSize(width(), height());

  Timer::callAfterDelay(0, [this] { _regex.grabKeyboardFocus(); });
}

int FindDialog::width() {
  return CONTENT_WIDTH + SPACE * 2;
}

int FindDialog::height() {
  return CONTENT_HEIGHT + SPACE * 2;
}

void FindDialog::resized() {
  auto area = getLocalBounds();
  area.reduce(SPACE, SPACE);

  _regex_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  _regex.setBounds(area.removeFromTop(DATA_ROW_HEIGHT));
  area.removeFromTop(BETWEEN_ROW_SPACE);

  auto row_area = area.removeFromTop(BUTTON_HEIGHT);
  _ok.setBounds(row_area.removeFromRight(BUTTON_WIDTH));
  row_area.removeFromRight(SPACE);
  _cancel.setBounds(row_area.removeFromRight(BUTTON_WIDTH));
}

void FindDialog::ok() {
  auto regex = _regex.getText();
  auto cursor = KeyMaster_instance()->cursor();
  if (_type == FindSong)
    cursor->goto_song(regex);
  else
    cursor->goto_set_list(regex);

  static_cast<DialogWindow*>(getParentComponent())->closeButtonPressed();
}

void FindDialog::cancel() {
  static_cast<DialogWindow*>(getParentComponent())->closeButtonPressed();
}
