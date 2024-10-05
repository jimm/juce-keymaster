#include "trigger_editor.h"
#include "../km/consts.h"
#include "../km/keymaster.h"
#include "../km/trigger.h"
#include "../km/editor.h"
#include "../km/utils.h"

#define FIELD_WIDTH 200
#define INPUT_MESSAGE_LABEL_WIDTH 60
#define INPUT_MESSAGE_WIDTH 240
#define LEARN_BUTTON_WIDTH 60

#define CONTENT_WIDTH (INPUT_MESSAGE_LABEL_WIDTH + SPACE + INPUT_MESSAGE_WIDTH + SPACE + LEARN_BUTTON_WIDTH)
#define CONTENT_HEIGHT (SPACE * 4 + BETWEEN_ROW_SPACE * 2 + LABEL_HEIGHT * 3 + DATA_ROW_HEIGHT * 4)

#define MSG_ID_OFFSET 100

TriggerEditor * open_trigger_editor(Trigger *t)
{
  bool is_new = t == nullptr;
  if (is_new) {
    Editor e;
    t = e.create_trigger();
  }

  DialogWindow::LaunchOptions opts;
  opts.dialogTitle = "Trigger";
  opts.dialogBackgroundColour =
    LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
  opts.resizable = false;
  auto cdc = new TriggerEditor(t, is_new);
  opts.content.setOwned(cdc);
  auto dialog_win = opts.launchAsync();
  if (dialog_win != nullptr)
    dialog_win->centreWithSize(cdc->width(), cdc->height());
  return cdc;
}

TriggerEditor::TriggerEditor(Trigger *t, bool is_new)
  : KmEditor(is_new), MidiMessageLearner(false, false, true, false), _trigger(t)
{
  init();
  addActionListener(this);
}

int TriggerEditor::width() {
  return KmEditor::width() + CONTENT_WIDTH;
}

int TriggerEditor::height() {
  return KmEditor::height() + CONTENT_HEIGHT;
}

void TriggerEditor::layout(Rectangle<int> &area) {
  layout_key(area);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_input_and_message(area);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_action(area);

  KmEditor::layout(area);
}

void TriggerEditor::layout_key(Rectangle<int> &area) {
  _key_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _key.setBounds(row_area.removeFromLeft(FIELD_WIDTH));
}

void TriggerEditor::layout_input_and_message(Rectangle<int> &area) {
  _input_and_message_label.setBounds(area.removeFromTop(LABEL_HEIGHT));

  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _input_label.setBounds(row_area.removeFromLeft(INPUT_MESSAGE_LABEL_WIDTH));
  row_area.removeFromLeft(SPACE);
  _input.setBounds(row_area.removeFromLeft(FIELD_WIDTH));

  area.removeFromTop(SPACE);
  row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _message_label.setBounds(row_area.removeFromLeft(INPUT_MESSAGE_LABEL_WIDTH));
  row_area.removeFromLeft(SPACE);
  _message.setBounds(row_area.removeFromLeft(INPUT_MESSAGE_WIDTH));

  row_area.removeFromLeft(SPACE);
  _learn.setBounds(row_area.removeFromLeft(LEARN_BUTTON_WIDTH));
}

void TriggerEditor::layout_action(Rectangle<int> &area) {
  _action_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _action.setBounds(row_area.removeFromLeft(FIELD_WIDTH));
}

void TriggerEditor::init() {
  init_key();
  init_input_and_message();
  init_action();

  KmEditor::init();
}

void TriggerEditor::init_key() {
  _key.setText(
    _trigger->has_trigger_key_press()
      ? _trigger->trigger_key_press().getTextDescription()
      : "(Not set)",
    NotificationType::dontSendNotification
  );

  addAndMakeVisible(_key_label);
  addAndMakeVisible(_key);
}

void TriggerEditor::init_input_and_message() {
  _input.addItem("(No input selected)", UNSELECTED);
  int i = 0;
  for (auto inp : KeyMaster_instance()->device_manager().inputs()) {
    _input.addItem(inp->info.name, i+1);
    if (inp == _trigger->trigger_input())
      _input.setSelectedId(i);
    ++i;
  }
  if (_input.getSelectedId() == 0)
    _input.setSelectedId(UNSELECTED);

  _input_trigger_message = _trigger->trigger_message();
  draw_input_message();
  _learn.onClick = [this] {
    auto inp = selected_input();
    if (inp != nullptr) {
      _learn.setButtonText("Listening...");
      inp->learn(this);
    }
  };

  addAndMakeVisible(_input_and_message_label);
  addAndMakeVisible(_input_label);
  addAndMakeVisible(_input);
  addAndMakeVisible(_message_label);
  addAndMakeVisible(_message);
  addAndMakeVisible(_learn);
}
  
void TriggerEditor::init_action() {
  _action.addItem("Next Song", TriggerAction::NEXT_SONG + 1);
  _action.addItem("Prev Song", TriggerAction::PREV_SONG + 1);
  _action.addItem("Next Patch", TriggerAction::NEXT_PATCH + 1);
  _action.addItem("Prev Patch", TriggerAction::PREV_PATCH + 1);
  _action.addItem("Panic", TriggerAction::PANIC + 1);
  _action.addItem("Super Panic", TriggerAction::SUPER_PANIC + 1);
  _action.addItem("Toggle Clock On/Off", TriggerAction::TOGGLE_CLOCK + 1);

  if (!KeyMaster_instance()->messages().isEmpty()) {
    _action.addSeparator();
    int i = MSG_ID_OFFSET;
    for (auto msg : KeyMaster_instance()->messages()) {
      _action.addItem(msg->name(), i);
      if (msg == _trigger->output_message())
        _action.setSelectedId(i);
      ++i;
    }
  }
  if (_trigger->action() != TriggerAction::MESSAGE)
    _action.setSelectedId(_trigger->action() + 1);
  // messages already handled when adding messages to the menu

  addAndMakeVisible(_action_label);
  addAndMakeVisible(_action);
}

void TriggerEditor::cancel_cleanup() {
  delete _trigger;
}

bool TriggerEditor::apply() {
  auto text = _key.getText();
  KeyPress kp { UNDEFINED };
  if (!text.isEmpty() && text != "(Not set)") {
    auto temp_kp = KeyPress::createFromDescription(_key.getText());
    kp = temp_kp;
  }

  Input::Ptr inp = selected_input();

  TriggerAction action;
  int action_id = _action.getSelectedId();
  MessageBlock *output_message = nullptr;
  if (action_id >= MSG_ID_OFFSET) {
    action = TriggerAction::MESSAGE;
    output_message = KeyMaster_instance()->messages()[action_id - MSG_ID_OFFSET];
  }
  else
    action = static_cast<TriggerAction>(action_id - 1);

  _trigger->set_trigger_key_press(kp);
  _trigger->set_trigger_message(inp, _input_trigger_message);
  _trigger->set_action(action);
  _trigger->set_output_message(output_message);

  if (_is_new) {
    Editor e;
    e.add_trigger(_trigger);
    _is_new = false;
  }

  sendActionMessage("update:table-list-box");
  return true;
}

Input::Ptr TriggerEditor::selected_input() {
  int inp_id = _input.getSelectedId();
  if (inp_id >= 1)
    return KeyMaster_instance()->device_manager().inputs()[inp_id - 1];
  return nullptr;
}

void TriggerEditor::draw_input_message() {
  String text = mm_equal(_input_trigger_message, EMPTY_MESSAGE)
    ? "(None)"
    : _input_trigger_message.getDescription();
  _message.setText(text, NotificationType::dontSendNotification);
}

void TriggerEditor::learnMidiMessage(const MidiMessage &message) {
  _input_trigger_message = message;
  sendActionMessage("learning:done");
}

void TriggerEditor::actionListenerCallback(const String &message) {
  if (message == "learning:done") {
    draw_input_message();
    _learn.setButtonText("Learn");
  }
}
