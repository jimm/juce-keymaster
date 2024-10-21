#include "trigger_editor.h"
#include "../km/consts.h"
#include "../km/keymaster.h"
#include "../km/trigger.h"
#include "../km/editor.h"
#include "../km/formatter.h"
#include "../km/utils.h"

#define FIELD_WIDTH 200
#define KEY_LEARNER_WIDTH 60
#define KEY_LEARNER_BUTTON_WIDTH 100
#define INPUT_MESSAGE_LABEL_WIDTH 60
#define INPUT_MESSAGE_WIDTH 240
#define LEARN_BUTTON_WIDTH 60

#define CONTENT_WIDTH (INPUT_MESSAGE_LABEL_WIDTH + SPACE + INPUT_MESSAGE_WIDTH + SPACE + LEARN_BUTTON_WIDTH)
#define CONTENT_HEIGHT (SPACE * 3 + BETWEEN_ROW_SPACE * 2 + LABEL_HEIGHT * 3 + DATA_ROW_HEIGHT * 3)

#define MSG_ID_OFFSET 100

KeyLearner::KeyLearner() : Label("", ""), _learning(false) {
}

void KeyLearner::start_learning() {
  _learning = true;
  setWantsKeyboardFocus(true);
  enterModalState();
}

void KeyLearner::set_key(const KeyPress &key) {
  _key = key;
  setText(_key.isValid() ? _key.getTextDescriptionWithIcons() : "(None)",
          NotificationType::dontSendNotification);
}

bool KeyLearner::keyPressed(const KeyPress &key) {
  if (!_learning)
    return false;

  _learning = false;
  exitModalState();
  setWantsKeyboardFocus(false);
  set_key(key);
  sendActionMessage("key:learned");
  return true;
}

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
: KmEditor(is_new), MidiMessageLearner(true), _trigger(t)
{
  init();
  addActionListener(this);
}

TriggerEditor::~TriggerEditor() {
  stop_learning();
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
  layout_message(area);

  area.removeFromTop(BETWEEN_ROW_SPACE);
  layout_action(area);

  KmEditor::layout(area);
}

void TriggerEditor::layout_key(Rectangle<int> &area) {
  _key_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);

  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _key.setBounds(row_area.removeFromLeft(KEY_LEARNER_WIDTH));
  row_area.removeFromLeft(SPACE);
  _key_learn.setBounds(row_area.removeFromLeft(KEY_LEARNER_BUTTON_WIDTH));
  row_area.removeFromLeft(SPACE);
  _key_clear.setBounds(row_area.removeFromLeft(KEY_LEARNER_BUTTON_WIDTH));
}

void TriggerEditor::layout_message(Rectangle<int> &area) {
  _message_label.setBounds(area.removeFromTop(LABEL_HEIGHT));

  area.removeFromTop(SPACE);

  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _message.setBounds(row_area.removeFromLeft(INPUT_MESSAGE_WIDTH));
  row_area.removeFromLeft(SPACE);
  _message_learn.setBounds(row_area.removeFromLeft(LEARN_BUTTON_WIDTH));
  row_area.removeFromLeft(SPACE);
  _message_clear.setBounds(row_area.removeFromLeft(LEARN_BUTTON_WIDTH));
}

void TriggerEditor::layout_action(Rectangle<int> &area) {
  _action_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _action.setBounds(row_area.removeFromLeft(FIELD_WIDTH));
}

void TriggerEditor::init() {
  init_key();
  init_message();
  init_action();

  KmEditor::init();
}

void TriggerEditor::init_key() {
  _key.set_key(_trigger->trigger_key_press());
  _key.addActionListener(this);

  _key_learn.onClick = [this] {
    _key_learn.setButtonText("Press a Key...");
    _key.start_learning();
  };

  _key_clear.onClick = [this] {
    _key.set_key(KeyPress());
    _key_clear.setEnabled(false);
  };

  _key_clear.setEnabled(_key.key().isValid());

  addAndMakeVisible(_key_label);
  addAndMakeVisible(_key);
  addAndMakeVisible(_key_learn);
  addAndMakeVisible(_key_clear);
}

void TriggerEditor::init_message() {
  _input_trigger_message = _trigger->trigger_message();
  draw_input_message();
  _message_learn.onClick = [this] {
    if (is_learning())          // want to cancel
      stop_learning();
    else {
      _message_learn.setButtonText("Cancel");
      start_learning(1, [this] {
        sendActionMessage("message-learning:done");
      });
    }
  };

  _message_clear.onClick = [this] {
    _input_trigger_message = EMPTY_MESSAGE;
    _message_clear.setEnabled(false);
    sendActionMessage("message-learning:done");
  };
  _message_clear.setEnabled(!mm_equal(_input_trigger_message, EMPTY_MESSAGE));

  addAndMakeVisible(_message_label);
  addAndMakeVisible(_message);
  addAndMakeVisible(_message_learn);
  addAndMakeVisible(_message_clear);
}
  
void TriggerEditor::init_action() {
  _action.addItem("Next Song", TriggerAction::NEXT_SONG + 1);
  _action.addItem("Prev Song", TriggerAction::PREV_SONG + 1);
  _action.addItem("Next Patch", TriggerAction::NEXT_PATCH + 1);
  _action.addItem("Prev Patch", TriggerAction::PREV_PATCH + 1);
  _action.addItem("Panic", TriggerAction::PANIC + 1);
  _action.addItem("Super Panic", TriggerAction::SUPER_PANIC + 1);
  _action.addItem("Toggle Clock On/Off", TriggerAction::TOGGLE_CLOCK + 1);

  int selected_message_id = -1;
  if (!KeyMaster_instance()->messages().isEmpty()) {
    PopupMenu submenu;
    int i = MSG_ID_OFFSET;

    for (auto msg : KeyMaster_instance()->messages()) {
      bool is_selected = msg == _trigger->output_message();
      submenu.addItem(i, msg->name(), true, is_selected);
      if (is_selected)
        selected_message_id = i;
      ++i;
    }

    _action.getRootMenu()->addSubMenu("Send Message", submenu);
  }

  if (_trigger->action() == TriggerAction::MESSAGE)
    _action.setSelectedId(selected_message_id);
  else
    _action.setSelectedId(_trigger->action() + 1);

  addAndMakeVisible(_action_label);
  addAndMakeVisible(_action);
}

void TriggerEditor::cancel_cleanup() {
  delete _trigger;
}

bool TriggerEditor::apply() {
  TriggerAction action;
  int action_id = _action.getSelectedId();
  MessageBlock *output_message = nullptr;
  if (action_id >= MSG_ID_OFFSET) {
    action = TriggerAction::MESSAGE;
    output_message = KeyMaster_instance()->messages()[action_id - MSG_ID_OFFSET];
  }
  else
    action = static_cast<TriggerAction>(action_id - 1);

  _trigger->set_trigger_key_press(_key.key());
  _trigger->set_trigger_message(_input_trigger_message);
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

void TriggerEditor::draw_input_message() {
  String text = trigger_message_description(_input_trigger_message);
  _message.setText(text, NotificationType::dontSendNotification);
}

void TriggerEditor::actionListenerCallback(const String &message) {
  if (message == "message-learning:done") {
    _input_trigger_message = midi_messages()[0];
    draw_input_message();
    _message_learn.setButtonText("Learn");
    _message_clear.setEnabled(!mm_equal(_input_trigger_message, EMPTY_MESSAGE));
  }
  else if (message == "key:learned") {
    _key_learn.setButtonText("Set Key");
    _key_clear.setEnabled(_key.key().isValid());
  }
}
