#include "song_editor.h"
#include "../km/consts.h"
#include "../km/keymaster.h"
#include "../km/song.h"
#include "../km/editor.h"

#define NAME_WIDTH 300
#define BPM_LABEL_WIDTH 40
#define BPM_WIDTH 40
#define CLOCK_ON_CHECKBOX_WIDTH 150
#define NOTES_WIDTH 300
#define NOTES_HEIGHT 300

#define CONTENT_WIDTH (NOTES_WIDTH)
#define CONTENT_HEIGHT (SPACE * 3 + BETWEEN_ROW_SPACE * 3 + LABEL_HEIGHT * 3 + DATA_ROW_HEIGHT * 2 + NOTES_HEIGHT)

SongEditor * open_song_editor(Song *s)
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
  auto cdc = new SongEditor(s, is_new);
  opts.content.setOwned(cdc);
  auto dialog_win = opts.launchAsync();
  if (dialog_win != nullptr)
    dialog_win->centreWithSize(cdc->width(), cdc->height());
  return cdc;
}

SongEditor::SongEditor(Song *s, bool is_new)
  : KmEditor(is_new), _song(s)
{
  init();
}

int SongEditor::width() {
  return KmEditor::width() + CONTENT_WIDTH;
}

int SongEditor::height() {
  return KmEditor::height() + CONTENT_HEIGHT;
}

void SongEditor::layout(Rectangle<int> &area) {
  layout_name(area);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_clock(area);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_notes(area);

  KmEditor::layout(area);
}

void SongEditor::layout_name(Rectangle<int> &area) {
  _name_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _name.setBounds(row_area.removeFromLeft(NAME_WIDTH));
}

void SongEditor::layout_clock(Rectangle<int> &area) {
  _clock_label.setBounds(area.removeFromTop(LABEL_HEIGHT));

  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _bpm_label.setBounds(row_area.removeFromLeft(BPM_LABEL_WIDTH));
  row_area.removeFromLeft(SPACE);
  _bpm.setBounds(row_area.removeFromLeft(BPM_WIDTH));
  row_area.removeFromLeft(SPACE);
  _clock_on.setBounds(row_area.removeFromLeft(CLOCK_ON_CHECKBOX_WIDTH));
}

void SongEditor::layout_notes(Rectangle<int> &area) {
  _notes_label.setBounds(area.removeFromTop(LABEL_HEIGHT));

  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(NOTES_HEIGHT);
  _notes.setBounds(row_area.removeFromLeft(NOTES_WIDTH));
}

void SongEditor::init() {
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

  KmEditor::init();
}

void SongEditor::cancel_cleanup() {
  delete _song;
}

bool SongEditor::apply() {
  _song->set_name(_name.getText());
  _song->set_bpm(_bpm.getText().getFloatValue());
  _song->set_clock_on_at_start(_clock_on.getToggleState());
  _song->set_notes(_notes.getText());

  if (_is_new) {
    Editor e;
    e.add_song(_song);
    _is_new = false;
  }

  sendActionMessage("update:all");
  return true;
}
