#include "cc_map_dialog_component.h"
#include "../km/keymaster.h"
#include "../km/connection.h"
#include "../km/controller.h"

#define UNSELECTED (-1)

#define SPACE 12
#define BETWEEN_ROW_SPACE 20
#define LABEL_HEIGHT 16
#define BUTTON_HEIGHT 35
#define BUTTON_WIDTH 80
#define DATA_ROW_HEIGHT 20

#define CC_NUM_WIDTH 40
#define CC_MAP_LABEL_WIDTH 60
#define FILTER_TOGGLE_HEIGHT 20
#define DASH_WIDTH 10

#define WINDOW_WIDTH (SPACE + 300 + SPACE)
#define WINDOW_HEIGHT (SPACE * 6 + BETWEEN_ROW_SPACE * 5 + LABEL_HEIGHT * 4 + FILTER_TOGGLE_HEIGHT * 3 + DATA_ROW_HEIGHT * 3 + BUTTON_HEIGHT)

CcMapDialogComponent * open_cc_map_editor(Connection *conn, Controller *c)
{
  bool is_new = c == nullptr;
  if (is_new)
    c = new Controller(UNDEFINED, 0);

  DialogWindow::LaunchOptions opts;
  opts.dialogTitle = "Controller Mapping";
  opts.dialogBackgroundColour =
    LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
  opts.resizable = false;
  auto cmdc = new CcMapDialogComponent(conn, c, is_new);
  opts.content.setOwned(cmdc);
  auto dialog_win = opts.launchAsync();
  if (dialog_win != nullptr)
    dialog_win->centreWithSize(cmdc->width(), cmdc->height());
  return cmdc;
}

CcMapDialogComponent::CcMapDialogComponent(
  Connection *conn, Controller *c, bool is_new)
  : _conn(conn), _controller(c), _is_new(is_new)
{
  init();

  _ok.onClick = [this] { ok(); };
  _cancel.onClick = [this] { cancel(); };
  _apply.onClick = [this] { apply(); };

  addAndMakeVisible(_ok);
  addAndMakeVisible(_cancel);
  addAndMakeVisible(_apply);

  setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

int CcMapDialogComponent::width() {
  return WINDOW_WIDTH;
}

int CcMapDialogComponent::height() {
  return WINDOW_HEIGHT;
}

void CcMapDialogComponent::resized() {
  auto area = getLocalBounds();
  area.reduce(SPACE, SPACE);

  layout(area);
  area.removeFromTop(BETWEEN_ROW_SPACE * 2);
  layout_buttons(area);
}

void CcMapDialogComponent::layout(Rectangle<int> &area) {
  _cc_num_label.setBounds(area.removeFromTop(LABEL_HEIGHT));

  area.removeFromTop(SPACE);
  auto row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _cc_num.setBounds(row_area.removeFromLeft(CC_NUM_WIDTH));
  row_area.removeFromLeft(SPACE * 2);
  _translated_cc_num_label.setBounds(row_area.removeFromLeft(CC_MAP_LABEL_WIDTH));
  row_area.removeFromLeft(SPACE);
  _translated_cc_num.setBounds(row_area.removeFromLeft(CC_NUM_WIDTH));

  area.removeFromTop(BETWEEN_ROW_SPACE);
  _filters_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  _filtered.setBounds(area.removeFromTop(FILTER_TOGGLE_HEIGHT));
  _pass_through_0.setBounds(area.removeFromTop(FILTER_TOGGLE_HEIGHT));
  _pass_through_127.setBounds(area.removeFromTop(FILTER_TOGGLE_HEIGHT));

  area.removeFromTop(BETWEEN_ROW_SPACE);
  _min_max_in_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _min_in.setBounds(row_area.removeFromLeft(CC_NUM_WIDTH));
  row_area.removeFromLeft(SPACE);
  _dash1.setBounds(row_area.removeFromLeft(DASH_WIDTH));
  row_area.removeFromLeft(SPACE);
  _max_in.setBounds(row_area.removeFromLeft(CC_NUM_WIDTH));

  area.removeFromTop(BETWEEN_ROW_SPACE);
  _min_max_out_label.setBounds(area.removeFromTop(LABEL_HEIGHT));
  area.removeFromTop(SPACE);
  row_area = area.removeFromTop(DATA_ROW_HEIGHT);
  _min_out.setBounds(row_area.removeFromLeft(CC_NUM_WIDTH));
  row_area.removeFromLeft(SPACE);
  _dash2.setBounds(row_area.removeFromLeft(DASH_WIDTH));
  row_area.removeFromLeft(SPACE);
  _max_out.setBounds(row_area.removeFromLeft(CC_NUM_WIDTH));
}

void CcMapDialogComponent::layout_buttons(Rectangle<int> &area) {
  auto row_area = area.removeFromTop(BUTTON_HEIGHT);
  _ok.setBounds(row_area.removeFromRight(BUTTON_WIDTH));
  row_area.removeFromRight(SPACE);
  _apply.setBounds(row_area.removeFromRight(BUTTON_WIDTH));
  row_area.removeFromRight(SPACE);
  _cancel.setBounds(row_area.removeFromRight(BUTTON_WIDTH));
}

void CcMapDialogComponent::init() {
  _cc_num.setText(String(_controller->cc_num()));
  if (_controller->translated_cc_num() != UNDEFINED)
    _translated_cc_num.setText(String(_controller->translated_cc_num()));
  auto ignore = NotificationType::dontSendNotification;
  _filtered.setToggleState(_controller->filtered(), ignore);
  _pass_through_0.setToggleState(_controller->pass_through_0(), ignore);
  _pass_through_127.setToggleState(_controller->pass_through_127(), ignore);
  _min_in.setText(String(_controller->min_in()));
  _max_in.setText(String(_controller->max_in()));
  _min_out.setText(String(_controller->min_out()));
  _max_out.setText(String(_controller->max_out()));

  addAndMakeVisible(_cc_num_label);
  addAndMakeVisible(_cc_num);
  addAndMakeVisible(_translated_cc_num_label);
  addAndMakeVisible(_translated_cc_num);
  addAndMakeVisible(_filters_label);
  addAndMakeVisible(_filtered);
  addAndMakeVisible(_pass_through_0);
  addAndMakeVisible(_pass_through_127);
  addAndMakeVisible(_min_max_in_label);
  addAndMakeVisible(_min_in);
  addAndMakeVisible(_max_in);
  addAndMakeVisible(_min_max_out_label);
  addAndMakeVisible(_min_out);
  addAndMakeVisible(_max_out);
}

void CcMapDialogComponent::ok() {
  if (apply())
    static_cast<DialogWindow*>(getParentComponent())->closeButtonPressed();
}

void CcMapDialogComponent::cancel() {
  if (_is_new)
    delete _conn;
  static_cast<DialogWindow*>(getParentComponent())->closeButtonPressed();
}

bool CcMapDialogComponent::apply() {
  Array<String> error_msgs;

  auto text = _cc_num.getText();
  int cc_num = 0;
  if (text.isEmpty())
    error_msgs.add("Controller number must be defined");
  else
    cc_num = text.getIntValue();

  int translated_cc_num;
  text = _translated_cc_num.getText();
  translated_cc_num = text.isEmpty() ? UNDEFINED : text.getIntValue();

  bool filtered = _filtered.getToggleState();
  bool pass_0 = _pass_through_0.getToggleState();
  bool pass_127 = _pass_through_127.getToggleState();

  int min_in = _min_in.getText().getIntValue();
  if (min_in < 0 || min_in > 127)
    error_msgs.add("Min in must be 0 - 127");
  int max_in = _max_in.getText().getIntValue();
  if (max_in < 0 || max_in > 127)
    error_msgs.add("Max in must be 0 - 127");
  int min_out = _min_out.getText().getIntValue();
  if (min_out < 0 || min_out > 127)
    error_msgs.add("Min out must be 0 - 127");
  int max_out = _max_out.getText().getIntValue();
  if (max_out < 0 || max_out > 127)
    error_msgs.add("Max out must be 0 - 127");

  if (!error_msgs.isEmpty()) {
    String message = "The following";
    message << String((error_msgs.size() == 1) ? "error prevents" : "errors prevent");
    message << " the controller mapping from being saved:\n";
    for (auto err : error_msgs) {
      message << "\n- ";
      message << err;
    }
    AlertWindow::showMessageBoxAsync(MessageBoxIconType::WarningIcon,
                                     "Connection Edit Errors", message,
                                     "OK", this);
    return false;
  }

  _conn->begin_changes();

  _controller->set_cc_num(cc_num);
  _controller->set_translated_cc_num(translated_cc_num);
  _controller->set_filtered(filtered);
  _controller->set_range(pass_0, pass_127, min_in, max_in, min_out, max_out);

  _conn->set_cc_map(_controller->cc_num(), _controller);
  _conn->end_changes();

  _is_new = false;
  sendActionMessage(CC_MAP_CHANGED_MESSAGE);
  return true;
}
