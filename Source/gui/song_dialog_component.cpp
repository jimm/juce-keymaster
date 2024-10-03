#include "song_dialog_component.h"
#include "../km/consts.h"
#include "../km/keymaster.h"
#include "../km/song.h"
#include "../km/editor.h"

#define UNSELECTED (-1)

#define SPACE 12
#define BETWEEN_ROW_SPACE 20
#define LABEL_HEIGHT 16
#define BUTTON_HEIGHT 35
#define BUTTON_WIDTH 80
#define DATA_ROW_HEIGHT 20

#define NAME_WIDTH 300
#define BPM_LABEL_WIDTH 40
#define BPM_WIDTH 40
#define CLOCK_ON_CHECKBOX_WIDTH 150

#define NOTES_WIDTH 300
#define NOTES_HEIGHT 300

#define WINDOW_WIDTH (SPACE * 2 + NOTES_WIDTH)
#define WINDOW_HEIGHT (SPACE * 5 + BETWEEN_ROW_SPACE * 3 + LABEL_HEIGHT * 3 + DATA_ROW_HEIGHT * 2 + NOTES_HEIGHT + BUTTON_HEIGHT)

SongDialogComponent * open_song_editor(Song *s)
{
  bool is_new = s == nullptr;
  if (is_new) {
    Editor e;
    s = e.create_song();
  }

  DialogWindow::LaunchOptions opts;
  opts.dialogTitle = "Song";
  opts.dialogBackgroundColour =
    LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
  opts.resizable = false;
  auto cdc = new SongDialogComponent(s, is_new);
  opts.content.setOwned(cdc);
  auto dialog_win = opts.launchAsync();
  if (dialog_win != nullptr)
    dialog_win->centreWithSize(cdc->width(), cdc->height());
  return cdc;
}

SongDialogComponent::SongDialogComponent(
  Song *s, bool is_new)
  : _song(s), _is_new(is_new)
{
  init_song();

  _ok.onClick = [this] { ok(); };
  _cancel.onClick = [this] { cancel(); };
  _apply.onClick = [this] { apply(); };

  addAndMakeVisible(_ok);
  addAndMakeVisible(_cancel);
  addAndMakeVisible(_apply);

  setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

int SongDialogComponent::width() {
  return WINDOW_WIDTH;
}

int SongDialogComponent::height() {
  return WINDOW_HEIGHT;
}

void SongDialogComponent::resized() {
  auto area = getLocalBounds();
  area.reduce(SPACE, SPACE);

  layout_name(area);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_clock(area);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_notes(area);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_buttons(area);
}

void SongDialogComponent::layout_name(Rectangle<int> &area) {
  _name_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _name.setBounds(row_area.removeFromLeft(NAME_WIDTH));
}

void SongDialogComponent::layout_clock(Rectangle<int> &area) {
  _clock_label.setBounds(area.removeFromTop(LABEL_HEIGHT));

  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _bpm_label.setBounds(row_area.removeFromLeft(BPM_LABEL_WIDTH));
  row_area.removeFromLeft(SPACE);
  _bpm.setBounds(row_area.removeFromLeft(BPM_WIDTH));
  row_area.removeFromLeft(SPACE);
  _clock_on.setBounds(row_area.removeFromLeft(CLOCK_ON_CHECKBOX_WIDTH));
}

void SongDialogComponent::layout_notes(Rectangle<int> &area) {
  _notes_label.setBounds(area.removeFromTop(LABEL_HEIGHT));

  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(NOTES_HEIGHT);
  _notes.setBounds(row_area.removeFromLeft(NOTES_WIDTH));
}

void SongDialogComponent::layout_buttons(Rectangle<int> &area) {
  auto row_area = area.removeFromTop(BUTTON_HEIGHT);
  _ok.setBounds(row_area.removeFromRight(BUTTON_WIDTH));
  row_area.removeFromRight(SPACE);
  _apply.setBounds(row_area.removeFromRight(BUTTON_WIDTH));
  row_area.removeFromRight(SPACE);
  _cancel.setBounds(row_area.removeFromRight(BUTTON_WIDTH));
}

void SongDialogComponent::init_song() {
  _name.setText(_song->name());
  _bpm.setText(String(_song->bpm()));
  _clock_on.setToggleState(_song->clock_on_at_start(), NotificationType::dontSendNotification);
  _notes.setText(_song->notes());

  addAndMakeVisible(_name_label);
  addAndMakeVisible(_name);
  addAndMakeVisible(_clock_label);
  addAndMakeVisible(_bpm_label);
  addAndMakeVisible(_bpm);
  addAndMakeVisible(_clock_on);
  addAndMakeVisible(_notes_label);
  addAndMakeVisible(_notes);
}

void SongDialogComponent::ok() {
  if (apply())
    static_cast<DialogWindow*>(getParentComponent())->closeButtonPressed();
}

void SongDialogComponent::cancel() {
  if (_is_new)
    delete _song;
  static_cast<DialogWindow*>(getParentComponent())->closeButtonPressed();
}

// Displays an alert and returns false if apply fails.
bool SongDialogComponent::apply() {
  _song->set_name(_name.getText());
  _song->set_bpm(_bpm.getText().getFloatValue());
  _song->set_clock_on_at_start(_clock_on.getToggleState());
  _song->set_notes(_notes.getText());

  if (_is_new) {
    Editor e;
    e.add_song(_song);
    _is_new = false;
  }

  sendActionMessage(SONG_CHANGED_MESSAGE);
  return true;
}
