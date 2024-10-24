#include "message_editor.h"
#include "../km/consts.h"
#include "../km/keymaster.h"
#include "../km/song.h"
#include "../km/editor.h"
#include "../km/formatter.h"

#define NAME_WIDTH 300
#define BYTES_WIDTH 300
#define BYTES_HEIGHT 300
#define LEARN_BUTTON_WIDTH 60

#define CONTENT_WIDTH (BYTES_WIDTH)
#define CONTENT_HEIGHT (SPACE * 3 + BETWEEN_ROW_SPACE + LABEL_HEIGHT * 2 + DATA_ROW_HEIGHT * 2 + BYTES_HEIGHT)

MessageEditor * open_message_editor(MessageBlock *m)
{
  bool is_new = m == nullptr;
  if (is_new) {
    Editor e;
    m = e.create_message();
  }

  DialogWindow::LaunchOptions opts;
  opts.dialogTitle = "Message";
  opts.dialogBackgroundColour =
    LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
  opts.resizable = false;
  auto cdc = new MessageEditor(m, is_new);
  opts.content.setOwned(cdc);
  auto dialog_win = opts.launchAsync();
  if (dialog_win != nullptr)
    dialog_win->centreWithSize(cdc->width(), cdc->height());
  return cdc;
}

MessageEditor::MessageEditor(MessageBlock *m, bool is_new)
  : KmEditor(is_new), MidiMessageLearner(true), _message(m)
{
  init();
}

MessageEditor::~MessageEditor() {
  stop_learning();
}

int MessageEditor::width() {
  return KmEditor::width() + CONTENT_WIDTH;
}

int MessageEditor::height() {
  return KmEditor::height() + CONTENT_HEIGHT;
}

void MessageEditor::layout(Rectangle<int> &area) {
  layout_name(area);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_bytes(area);

  KmEditor::layout(area);
}

void MessageEditor::layout_name(Rectangle<int> &area) {
  _name_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _name.setBounds(row_area.removeFromLeft(NAME_WIDTH));
}

void MessageEditor::layout_bytes(Rectangle<int> &area) {
  _bytes_label.setBounds(area.removeFromTop(LABEL_HEIGHT));

  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _bytes_clear.setBounds(row_area.removeFromRight(LEARN_BUTTON_WIDTH));
  row_area.removeFromRight(SPACE);
  _bytes_learn.setBounds(row_area.removeFromRight(LEARN_BUTTON_WIDTH));

  area.removeFromTop(SPACE);
  row_area = area.removeFromTop(BYTES_HEIGHT);
  _bytes.setBounds(row_area.removeFromLeft(BYTES_WIDTH));
}

void MessageEditor::init() {
  addActionListener(this);

  _name.setText(_message->name());
  _bytes.setMultiLine(true);
  _bytes.setReturnKeyStartsNewLine(true);
  _bytes.setCaretVisible(true);
  _bytes.setText(_message->to_editable_hex_string());
  _bytes.addListener(this);

  _bytes_learn.onClick = [this] {
    if (is_learning()) {        // stop
      stop_learning();
      // _message.from_midi_messages(midi_messages());
      _bytes.setText(to_hex(midi_messages(), "\n"));
      _bytes.repaint();
      _bytes_learn.setButtonText("Learn");
    }
    else {
      _bytes.setText("");
      _bytes_learn.setButtonText("Stop");
      start_learning();
    }
    _bytes_clear.setEnabled(!midi_messages().isEmpty());
  };

  _bytes_clear.onClick = [this] {
    midi_messages().clear();
    _bytes.setText("");
    _bytes_clear.setEnabled(false);
  };

  _bytes_clear.setEnabled(!_message->midi_messages().isEmpty());

  addAndMakeVisible(_name_label);
  addAndMakeVisible(_name);
  addAndMakeVisible(_bytes_label);
  addAndMakeVisible(_bytes_learn);
  addAndMakeVisible(_bytes_clear);
  addAndMakeVisible(_bytes);

  KmEditor::init();
}

void MessageEditor::cancel_cleanup() {
  delete _message;
}

bool MessageEditor::apply() {
  _message->set_name(_name.getText());
  auto result = _message->from_hex_string(_bytes.getText());
  if (result.failed()) {
    error_messages.add(result.getErrorMessage());
    display_errors("Message");
    return false;
  }

  if (_is_new) {
    Editor e;
    e.add_message(_message);
    _is_new = false;
  }

  sendActionMessage("update:list-box");
  return true;
}

void MessageEditor::learn_midi_message(const MidiMessage &message) {
  MidiMessageLearner::learn_midi_message(message);
  sendActionMessage("message:learned");
}

void MessageEditor::actionListenerCallback(const String &message) {
  if (message == "message:learned")
    _bytes.setText(to_hex(midi_messages(), "\n"));
}

void MessageEditor::textEditorTextChanged(TextEditor &) {
  _bytes_clear.setEnabled(_bytes.getText().containsNonWhitespaceChars());
}
